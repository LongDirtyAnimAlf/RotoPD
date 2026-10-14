#ifndef _QUEUE_H_
#define _QUEUE_H_

//#pragma once          // or classic include guards

#include "Arduino.h"

#ifdef __cplusplus          // only compile the class when using a C++ compiler

template<typename T, size_t SIZE>
class CircularDataQueue {
    static_assert(SIZE <= 65535, "CircularDataQueue size exceeds uint16_t counter range");
private:
    T* frames;
    uint16_t head;
    uint16_t tail;
    uint16_t count;

public:
    CircularDataQueue() : head(0), tail(0), count(0) {
      frames = (T*)pmalloc(SIZE * sizeof(T));
    }

    ~CircularDataQueue() {
      if (frames) free(frames);
    }

    bool push(const T& item) {
        if (count >= SIZE) {
            // Virtual pop
            head = (head + 1) % SIZE;
            count--;
            //return false;
        }
        frames[tail] = item;
        tail = (tail + 1) % SIZE;
        count++;
        return true;
    }

    bool pop(T& item) {
        if (count == 0) {
            return false;
        }
        item = frames[head];
        head = (head + 1) % SIZE;
        count--;
        return true;
    }

    bool peek(T& item) const {
        if (count == 0) {
            return false;
        }
        item = frames[head];
        return true;
    }

    uint16_t getCount() const {
        return count;
    }

    bool isEmpty() const {
        return count == 0;
    }

    bool isFull() const {
        return count >= SIZE;
    }

    void clear() {
        head = 0;
        tail = 0;
        count = 0;
    }

    // Copies up to 'maxItems' of the most recent entries into dest array.
    // Returns the actual number of items copied.
    uint16_t peekLast(T* dest, uint16_t maxItems) const {
        if (count == 0 || dest == nullptr || maxItems == 0) {
            return 0;
        }

        // Do not copy more than available items
        uint16_t toCopy = (maxItems > count) ? count : maxItems;

        // Index of the oldest item among the last 'toCopy' items
        // (tail - toCopy + SIZE) % SIZE handles circular index wrapping
        uint16_t startIdx = (tail + SIZE - toCopy) % SIZE;

        for (uint16_t i = 0; i < toCopy; i++) {
            dest[i] = frames[(startIdx + i) % SIZE];
        }

        return toCopy;
    }

    // Copies up to 'maxItems' of the newest entries into dest array, starting with the absolute newest at index 0.
    // Returns the actual number of items copied.
    uint16_t peekNewest(T* dest, uint16_t maxItems) const {
        if (count == 0 || dest == nullptr || maxItems == 0) {
            return 0;
        }

        uint16_t toCopy = (maxItems > count) ? count : maxItems;

        for (uint16_t i = 0; i < toCopy; i++) {
            // Step backward from tail (tail - 1 is the latest inserted item)
            uint16_t idx = (tail + SIZE - 1 - i) % SIZE;
            dest[i] = frames[idx];
        }

        return toCopy;
    }    

    // Index 0 = newest item, Index 1 = 2nd newest, ... Index (count - 1) = oldest item
    bool peekFromNewest(uint16_t offsetFromNewest, T& item) const {
        if (offsetFromNewest >= count) {
            return false; // Out of bounds or queue is empty
        }

        uint16_t idx = (tail + SIZE - 1 - offsetFromNewest) % SIZE;
        item = frames[idx];
        return true;
    }

};

#endif /*__cplusplus*/

#endif /*_QUEUE_H_*/