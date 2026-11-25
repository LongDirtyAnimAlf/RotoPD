#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "Arduino.h"

#ifdef ARDUINO_ARCH_ESP32

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include <type_traits>
#include <algorithm>
#include <cstring>

#ifdef __cplusplus

/**
 * Ultra High-Performance Circular Data Queue Optimized for ESP32
 * - Power-of-2 sizing uses fast bitwise AND (&) instead of costly division
 * - Optional thread-safety via template parameter:
 *     CircularDataQueue<MyType, 64, true>  -> Safe for multi-core / ISR (uses portMUX spinlocks)
 *     CircularDataQueue<MyType, 64, false> -> Single-threaded lockless (Zero locking overhead)
 * - Automatic SPIRAM allocation detection with internal DRAM fallback
 * - Fast-path memcpy via std::is_trivially_copyable for POD types
 */
template<typename T, uint16_t CIRCULARBUFFERSIZE, bool ThreadSafe = true>
class CircularDataQueue {
    static_assert(CIRCULARBUFFERSIZE >= 1 && CIRCULARBUFFERSIZE <= 65536, "QueueCapacity must be between 1 and 65536");
    static_assert(((CIRCULARBUFFERSIZE & (CIRCULARBUFFERSIZE - 1)) == 0), "QueueCapacity must be a power of two (16, 32, 64, 128, etc.)");

private:
    static constexpr uint32_t MASK = static_cast<uint32_t>(CIRCULARBUFFERSIZE - 1);

    T*                  frames = nullptr;
    T                   lastItem{};
    uint16_t            head = 0;
    uint16_t            tail = 0;
    uint16_t            count = 0;
    bool                is_spiram = false;

    mutable portMUX_TYPE lock = portMUX_INITIALIZER_UNLOCKED;

    // Multi-core and ISR safe RAII Spinlock Guard (Conditionally compiled)
    class LockGuard {
        portMUX_TYPE* mux_;
    public:
        explicit LockGuard(portMUX_TYPE* mux) : mux_(mux) {
            if constexpr (ThreadSafe) {
                portENTER_CRITICAL_ISR(mux_);
            }
        }
        ~LockGuard() {
            if constexpr (ThreadSafe) {
                portEXIT_CRITICAL_ISR(mux_);
            }
        }
        LockGuard(const LockGuard&) = delete;
        LockGuard& operator=(const LockGuard&) = delete;
    };

public:
    CircularDataQueue() {
        if constexpr (ThreadSafe) {
            //vPortCPUInitializeMutex(&lock);
        }

        size_t totalBytes = CIRCULARBUFFERSIZE * sizeof(T);

        // For large buffers on supported boards, attempt SPIRAM allocation first
        #if defined(BOARD_HAS_PSRAM) || defined(CONFIG_SPIRAM_SUPPORT)
        if (totalBytes >= 512 && psramFound()) {
            frames = static_cast<T*>(heap_caps_malloc(totalBytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
            if (frames) is_spiram = true;
        }
        #endif

        // Standard internal heap fallback
        if (!frames) {
            frames = static_cast<T*>(heap_caps_malloc(totalBytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
        }

        // Final generic heap fallback
        if (!frames) {
            frames = static_cast<T*>(malloc(totalBytes));
        }
    }

    ~CircularDataQueue() {
        if (frames) {
            clear();
            if (is_spiram) {
                heap_caps_free(frames);
            } else {
                free(frames);
            }
            frames = nullptr;
        }
    }

    // Non-copyable / non-movable
    CircularDataQueue(const CircularDataQueue&) = delete;
    CircularDataQueue& operator=(const CircularDataQueue&) = delete;
    CircularDataQueue(CircularDataQueue&&) = delete;
    CircularDataQueue& operator=(CircularDataQueue&&) = delete;

    bool isValid() const { return frames != nullptr; }

    // ---------- Write Side ----------

    /** Blocking Push: Always succeeds (overwrites oldest entry if full) */
    bool push(const T& item) {
        return pushTimeout(item, UINT32_MAX, true);
    }

    /** Non-blocking Push: Rejects insertion if full */
    bool tryPush(const T& item) {
        return pushTimeout(item, 0, false);
    }

    /** Push with timeout (in milliseconds) */
    bool pushTimeout(const T& item, uint32_t timeout_ms, bool overwriteIfFull = false) {
        if (!isValid()) return false;

        TickType_t startTick = xTaskGetTickCount();
        TickType_t timeoutTicks = (timeout_ms == UINT32_MAX) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);

        while (true) {
            if constexpr (ThreadSafe) {
                portENTER_CRITICAL_ISR(&lock);
            }

            if (count >= CIRCULARBUFFERSIZE) {
                if (!overwriteIfFull) {
                    if constexpr (ThreadSafe) {
                        portEXIT_CRITICAL_ISR(&lock);
                    }

                    if (timeout_ms == 0) return false;
                    if (timeout_ms != UINT32_MAX && (xTaskGetTickCount() - startTick) >= timeoutTicks) {
                        return false; // Timed out waiting for available space
                    }

                    // Yield execution to allow reader tasks to consume items
                    if (xPortInIsrContext()) {
                        return false; // Cannot delay inside ISR
                    } else {
                        vTaskDelay(1);
                    }
                    continue;
                }
                head = (head + 1) & MASK; // Overwrite oldest item
            } else {
                ++count;
            }

            if constexpr (std::is_trivially_copyable<T>::value) {
                memcpy(&lastItem, &item, sizeof(T));
                memcpy(&frames[tail], &item, sizeof(T));
            } else {
                lastItem     = item;
                frames[tail] = item;
            }

            tail = (tail + 1) & MASK;

            if constexpr (ThreadSafe) {
                portEXIT_CRITICAL_ISR(&lock);
            }
            return true;
        }
    }

    // ---------- Read Side ----------

    bool pop(T& item) {
        if (!isValid()) return false;
        LockGuard g(&lock);

        if (count == 0) {
            item = T{};
            return false;
        }

        if constexpr (std::is_trivially_copyable<T>::value) {
            memcpy(&item, &frames[head], sizeof(T));
        } else {
            item = frames[head];
        }

        head = (head + 1) & MASK;
        --count;
        return true;
    }

    bool peek(T& item) const {
        if (!isValid()) return false;
        LockGuard g(&lock);

        if (count == 0) {
            item = T{};
            return false;
        }

        if constexpr (std::is_trivially_copyable<T>::value) {
            memcpy(&item, &frames[head], sizeof(T));
        } else {
            item = frames[head];
        }

        return true;
    }

    uint16_t getCount() const {
        if (!isValid()) return 0;
        LockGuard g(&lock);
        return count;
    }

    bool isEmpty() const { return getCount() == 0; }
    bool isFull()  const { return getCount() >= CIRCULARBUFFERSIZE; }

    void clear() {
        if (!isValid()) return;
        LockGuard g(&lock);
        head = tail = count = 0;
        lastItem = T{};
    }

    /** Oldest → newest (Chronological order) */
    uint16_t peekLast(T* dest, uint16_t maxItems) const {
        if (!dest || maxItems == 0 || !isValid()) return 0;
        LockGuard g(&lock);

        if (count == 0) return 0;

        uint16_t toCopy = std::min<uint16_t>(maxItems, count);
        uint32_t start  = (static_cast<uint32_t>(tail) + CIRCULARBUFFERSIZE - toCopy) & MASK;

        if constexpr (std::is_trivially_copyable<T>::value) {
            uint32_t firstChunk = std::min<uint32_t>(toCopy, CIRCULARBUFFERSIZE - start);
            memcpy(dest, &frames[start], firstChunk * sizeof(T));

            if (toCopy > firstChunk) {
                uint32_t secondChunk = toCopy - firstChunk;
                memcpy(dest + firstChunk, &frames[0], secondChunk * sizeof(T));
            }
        } else {
            for (uint16_t i = 0; i < toCopy; ++i) {
                dest[i] = frames[(start + i) & MASK];
            }
        }
        return toCopy;
    }

    /** Newest → older (Reverse chronological order) */
    uint16_t peekNewest(T* dest, uint16_t maxItems) const {
        if (!dest || maxItems == 0 || !isValid()) return 0;
        LockGuard g(&lock);

        if (count == 0) return 0;

        uint16_t toCopy = std::min<uint16_t>(maxItems, count);

        if constexpr (std::is_trivially_copyable<T>::value) {
            for (uint16_t i = 0; i < toCopy; ++i) {
                uint32_t idx = (static_cast<uint32_t>(tail) + CIRCULARBUFFERSIZE - 1 - i) & MASK;
                memcpy(&dest[i], &frames[idx], sizeof(T));
            }
        } else {
            for (uint16_t i = 0; i < toCopy; ++i) {
                uint32_t idx = (static_cast<uint32_t>(tail) + CIRCULARBUFFERSIZE - 1 - i) & MASK;
                dest[i] = frames[idx];
            }
        }
        return toCopy;
    }

    /** Index 0 = newest */
    bool peekFromNewest(uint16_t offsetFromNewest, T& item) const {
        if (!isValid()) return false;
        LockGuard g(&lock);

        if (offsetFromNewest >= count) return false;

        uint32_t idx = (static_cast<uint32_t>(tail) + CIRCULARBUFFERSIZE - 1 - offsetFromNewest) & MASK;

        if constexpr (std::is_trivially_copyable<T>::value) {
            memcpy(&item, &frames[idx], sizeof(T));
        } else {
            item = frames[idx];
        }

        return true;
    }

    bool removeIf(bool (*pred)(const T&)) {
        if (!pred || !isValid()) return false;
        LockGuard g(&lock);

        if (count == 0) return false;

        uint16_t read     = head;
        uint16_t write    = head;
        uint16_t newCount = 0;
        bool     removed  = false;

        for (uint16_t i = 0; i < count; ++i) {
            if (!removed && pred(frames[read])) {
                removed = true;
            } else {
                if (write != read) {
                    if constexpr (std::is_trivially_copyable<T>::value) {
                        memcpy(&frames[write], &frames[read], sizeof(T));
                    } else {
                        frames[write] = frames[read];
                    }
                }
                write = (write + 1) & MASK;
                ++newCount;
            }
            read = (read + 1) & MASK;
        }

        if (removed) {
            count = newCount;
            tail  = write;
        }
        return removed;
    }

    T getLastData() const {
        if (!isValid()) return T{};
        LockGuard g(&lock);
        if (count == 0) return T{};

        if constexpr (std::is_trivially_copyable<T>::value) {
            T temp;
            memcpy(&temp, &lastItem, sizeof(T));
            return temp;
        } else {
            return lastItem;
        }
    }
};

#endif /* __cplusplus */

#endif /* ARDUINO_ARCH_ESP32 */

#endif /* _QUEUE_H_ */