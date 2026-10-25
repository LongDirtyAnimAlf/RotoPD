#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "Arduino.h"
#include "pico/mutex.h"          // required for mutex

#ifdef __cplusplus

template<typename T, size_t SIZE>
class CircularDataQueue {
    static_assert(SIZE <= 65535, "CircularDataQueue size exceeds uint16_t counter range");

private:
    T* frames;
    T lastItem;
    uint16_t head;
    uint16_t tail;
    uint16_t count;

    mutable mutex_t queue_mutex;   // mutable so const methods can lock

public:
    CircularDataQueue() : head(0), tail(0), count(0) {
        mutex_init(&queue_mutex);
        frames = (T*)pmalloc(SIZE * sizeof(T));
    }

    ~CircularDataQueue() {
        if (frames) free(frames);
    }

    // ---------- write side ----------
    bool push(const T& item) {
        mutex_enter_blocking(&queue_mutex);

        if (count >= SIZE) {
            // overwrite oldest
            head = (head + 1) % SIZE;
            count--;
        }
        lastItem = item;
        frames[tail] = lastItem;
        tail = (tail + 1) % SIZE;
        count++;

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
        head = 0;
        tail = 0;
        count = 0;
        mutex_exit(&queue_mutex);
    }

    // Copies up to 'maxItems' of the most recent entries into dest array.
    // Returns the actual number of items copied.
    uint16_t peekLast(T* dest, uint16_t maxItems) const {
        if (dest == nullptr || maxItems == 0) return 0;

        mutex_enter_blocking(&queue_mutex);

        if (count == 0) {
            mutex_exit(&queue_mutex);
            return 0;
        }

        uint16_t toCopy = (maxItems > count) ? count : maxItems;
        uint16_t startIdx = (tail + SIZE - toCopy) % SIZE;

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
            uint16_t idx = (tail + SIZE - 1 - i) % SIZE;
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

        uint16_t idx = (tail + SIZE - 1 - offsetFromNewest) % SIZE;
        item = frames[idx];

        mutex_exit(&queue_mutex);
        return true;
    }

    // Remove the first element for which pred() returns true.
    // Preserves relative order of remaining elements.
    // Returns true if an element was removed.
    bool removeIf(bool (*pred)(const T&)) {
        mutex_enter_blocking(&queue_mutex);
        if (count == 0 || pred == nullptr) {
            return false;
        }

        uint16_t read  = head;
        uint16_t write = head;
        uint16_t newCount = 0;
        bool removed = false;

        for (uint16_t i = 0; i < count; i++) {
            if (!removed && pred(frames[read])) {
                // skip this element (first match only)
                removed = true;
            } else {
                if (write != read) {
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