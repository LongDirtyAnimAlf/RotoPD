#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "Arduino.h"
//#include <new> // Required for placement new

#if defined(ARDUINO_ARCH_ESP32)
  // --- ESP32 TRANSLATION LAYER ---
  typedef SemaphoreHandle_t mutex_t;

  // Translates RP2350 timeout locks to ESP32 FreeRTOS ticks
  inline bool mutex_enter_timeout_ms(mutex_t* mtx, uint32_t timeout_ms) {
      return (xSemaphoreTake(*mtx, pdMS_TO_TICKS(timeout_ms)) == pdTRUE);
  }

  // A blocking enter maps to waiting forever (portMAX_DELAY)
  inline void mutex_enter_blocking(mutex_t* mtx) {
      xSemaphoreTake(*mtx, portMAX_DELAY);
  }

  // Translates RP2350 unlocks to ESP32 FreeRTOS semaphore releases
  inline void mutex_exit(mutex_t* mtx) {
      xSemaphoreGive(*mtx);
  }

  // Translates RP2350 mutex creation to ESP32 FreeRTOS creation
  inline void mutex_init(mutex_t* mtx) {
      *mtx = xSemaphoreCreateMutex();
  }

#elif defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_RP2350)
  // --- NATIVE RP2350 SDK ---
  #include "pico/mutex.h"
#endif

#ifdef __cplusplus

template<typename T, size_t SIZE>
class CircularDataQueue {
    static_assert(SIZE <= 65535, "CircularDataQueue size exceeds uint16_t counter range");

private:
    T* frames = NULL;
    T lastItem;
    uint16_t head = 0;
    uint16_t tail = 0;
    uint16_t count = 0;

    // mutable so const methods can lock
    mutable mutex_t queue_mutex;

public:
    CircularDataQueue() : head(0), tail(0), count(0) {
        mutex_init(&queue_mutex);
        // Use standard malloc or generic pvPortMalloc/pmalloc depending on framework
        if (SIZE < 100) frames = (T*)malloc(SIZE * sizeof(T));
        #ifdef ARDUINO_ARCH_RP2040       
        if (frames == NULL) frames = (T*)pmalloc(SIZE * sizeof(T));
        #endif
        #if defined(ARDUINO_ARCH_ESP32)
        if (frames == NULL) frames = (T*)heap_caps_aligned_alloc(4, (SIZE * sizeof(T)) , MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
        #endif
    }

    ~CircularDataQueue() {
        if (frames) {
            // Explicitly destroy any remaining live objects before freeing memory
            clear();
            free(frames);
        }
    }

    // CRITICAL FIX: Prevent C++ from shallow-copying the mutex or raw pointers
    CircularDataQueue(const CircularDataQueue&) = delete;
    CircularDataQueue& operator=(const CircularDataQueue&) = delete;
    CircularDataQueue(CircularDataQueue&&) = delete;
    CircularDataQueue& operator=(CircularDataQueue&&) = delete;

    // ---------- write side ----------
    bool push(const T& item) {
        if (!frames) return false;

        mutex_enter_blocking(&queue_mutex);

        if (count >= SIZE) {
            // Explicitly call destructor of the item being overwritten
            //frames[head].~T();
            head = (head + 1) % SIZE;
        } else {
            count++;
        }
        
        lastItem = item;
        //::new (static_cast<void*>(&frames[tail])) T(item); // Use placement new to construct the object in memory safely
        frames[tail] = item; // Perfectly safe for POD structs
        tail = (tail + 1) % SIZE;

        mutex_exit(&queue_mutex);
        return true;
    }

    // ---------- read side ----------
    bool pop(T& item) {
        mutex_enter_blocking(&queue_mutex);

        if (count == 0) {
            mutex_exit(&queue_mutex);
            return false;
        }
        item = frames[head];
        // Explicitly destroy the popped object
        //frames[head].~T();
        head = (head + 1) % SIZE;
        count--;

        mutex_exit(&queue_mutex);
        return true;
    }

    bool peek(T& item) const {
        mutex_enter_blocking(&queue_mutex);

        if (count == 0) {
            mutex_exit(&queue_mutex);
            return false;
        }
        item = frames[head];

        mutex_exit(&queue_mutex);
        return true;
    }

    uint16_t getCount() const {
        mutex_enter_blocking(&queue_mutex);
        uint16_t c = count;
        mutex_exit(&queue_mutex);
        return c;
    }

    bool isEmpty() const {
        return getCount() == 0;
    }

    bool isFull() const {
        return getCount() >= SIZE;
    }

    void clear() {
        mutex_enter_blocking(&queue_mutex);
        // Call destructors for all live items remaining in the queue
        while (count > 0) {
            //frames[head].~T();
            head = (head + 1) % SIZE;
            count--;
        }
        head = 0;
        tail = 0;
        count = 0;
        mutex_exit(&queue_mutex);
    }

    // Copies up to 'maxItems' of the oldest available items up to the newest.
    uint16_t peekLast(T* dest, uint16_t maxItems) const {
        if (dest == nullptr || maxItems == 0) return 0;

        mutex_enter_blocking(&queue_mutex);

        if (count == 0) {
            mutex_exit(&queue_mutex);
            return 0;
        }

        uint16_t toCopy = (maxItems > count) ? count : maxItems;
        // FIX: Find the correct oldest index slice boundary relative to the current tail
        uint32_t startIdx = (static_cast<uint32_t>(tail) + SIZE - toCopy) % SIZE;

        for (uint16_t i = 0; i < toCopy; i++) {
            dest[i] = frames[(startIdx + i) % SIZE];
        }

        mutex_exit(&queue_mutex);
        return toCopy;
    }

    // Newest item at dest[0], then older ones.
    uint16_t peekNewest(T* dest, uint16_t maxItems) const {
        if (dest == nullptr || maxItems == 0) return 0;

        mutex_enter_blocking(&queue_mutex);

        if (count == 0) {
            mutex_exit(&queue_mutex);
            return 0;
        }

        uint16_t toCopy = (maxItems > count) ? count : maxItems;

        for (uint16_t i = 0; i < toCopy; i++) {
            uint32_t idx = (static_cast<uint32_t>(tail) + SIZE - 1 - i) % SIZE;
            dest[i] = frames[idx];
        }

        mutex_exit(&queue_mutex);
        return toCopy;
    }

    // Index 0 = newest, Index (count-1) = oldest
    bool peekFromNewest(uint16_t offsetFromNewest, T& item) const {
        mutex_enter_blocking(&queue_mutex);

        if (offsetFromNewest >= count) {
            mutex_exit(&queue_mutex);
            return false;
        }

        uint32_t idx = (static_cast<uint32_t>(tail) + SIZE - 1 - offsetFromNewest) % SIZE;
        item = frames[idx];

        mutex_exit(&queue_mutex);
        return true;
    }

    bool removeIf(bool (*pred)(const T&)) {
        mutex_enter_blocking(&queue_mutex);
        if (count == 0 || pred == nullptr) {
            mutex_exit(&queue_mutex);
            return false;
        }

        uint16_t read  = head;
        uint16_t write = head;
        uint16_t newCount = 0;
        bool removed = false;

        for (uint16_t i = 0; i < count; i++) {
            if (!removed && pred(frames[read])) {
                // skip this element (first match only) and call destructor
                //frames[read].~T();
                removed = true;
            } else {
                if (write != read) {
                    // Move using placement new and then destroy old location
                    //::new (static_cast<void*>(&frames[write])) T(frames[read]);
                    //frames[read].~T();
                    frames[write] = frames[read];
                }
                write = (write + 1) % SIZE;
                newCount++;
            }
            read = (read + 1) % SIZE;
        }

        if (removed) {
            count = newCount;
            tail  = write;
        }
        mutex_exit(&queue_mutex);
        return removed;
    }

    T getLastData() const {
        mutex_enter_blocking(&queue_mutex);
        T result = (count == 0) ? T{} : lastItem;
        mutex_exit(&queue_mutex);
        return result;
    }
};

#endif /* __cplusplus */

#endif /* _QUEUE_H_ */
