#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "Arduino.h"

#if defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_RP2350)

#include "pico/critical_section.h"
#include "pico/time.h"
#include <type_traits>
#include <algorithm>
#include <cstring>

#ifdef __cplusplus

/**
 * High-Performance Circular Data Queue Optimized for RP2040 / RP2350
 * - Power-of-2 sizing replaces costly modulo (%) with bitwise mask (&)
 * - Optional thread-safety via template parameter:
 *     CircularDataQueue<MyType, 64, true>  -> Safe for multi-core / IRQ (uses critical_section)
 *     CircularDataQueue<MyType, 64, false> -> Single-thread lockless (Zero locking overhead)
 * - Optimized memory copies for trivial types (PODs)
 */
template<typename T, uint16_t CIRCULARBUFFERSIZE, bool ThreadSafe = true>
class CircularDataQueue {
    static_assert(CIRCULARBUFFERSIZE >= 1 && CIRCULARBUFFERSIZE <= 65536, "QueueCapacity must be 1..65536");
    static_assert(((CIRCULARBUFFERSIZE & (CIRCULARBUFFERSIZE - 1)) == 0), "QueueCapacity must be a power of two (16, 32, 64, 128, etc.)");    

private:
    static constexpr uint32_t MASK = static_cast<uint32_t>(CIRCULARBUFFERSIZE - 1);

    T*                  frames = nullptr;
    T                   lastItem{};
    uint16_t            head = 0;
    uint16_t            tail = 0;
    uint16_t            count = 0;

    mutable critical_section_t lock;

    // RAII Guard that conditionally compiles lock execution
    class LockGuard {
        critical_section_t* cs_;
    public:
        explicit LockGuard(critical_section_t* cs) : cs_(cs) {
            if constexpr (ThreadSafe) {
                critical_section_enter_blocking(cs_);
            }
        }
        ~LockGuard() {
            if constexpr (ThreadSafe) {
                critical_section_exit(cs_);
            }
        }
        LockGuard(const LockGuard&) = delete;
        LockGuard& operator=(const LockGuard&) = delete;
    };

public:
    CircularDataQueue() {
        if constexpr (ThreadSafe) {
            critical_section_init(&lock);
        }

        if (CIRCULARBUFFERSIZE < 100) {
            frames = (T*)malloc(CIRCULARBUFFERSIZE * sizeof(T));
        }
        if (frames == nullptr) {
            frames = (T*)pmalloc(CIRCULARBUFFERSIZE * sizeof(T));
        }
        if (frames == nullptr) {
            frames = (T*)malloc(CIRCULARBUFFERSIZE * sizeof(T)); // Final fallback
        }
    }

    ~CircularDataQueue() {
        if (frames) {
            clear();
            free(frames);
            frames = nullptr;
        }

        if constexpr (ThreadSafe) {
            critical_section_deinit(&lock);
        }
    }

    // Non-copyable / non-movable
    CircularDataQueue(const CircularDataQueue&) = delete;
    CircularDataQueue& operator=(const CircularDataQueue&) = delete;
    CircularDataQueue(CircularDataQueue&&) = delete;
    CircularDataQueue& operator=(CircularDataQueue&&) = delete;

    bool isValid() const { return frames != nullptr; }

    // ---------- Write Side ----------

    bool push(const T& item) {
        return pushTimeout(item, UINT32_MAX, true);
    }

    bool tryPush(const T& item) {
        return pushTimeout(item, 0, false);
    }

    bool pushTimeout(const T& item, uint32_t timeout_ms, bool overwriteIfFull = false) {
        if (!isValid()) return false;

        absolute_time_t deadline = (timeout_ms == UINT32_MAX)
                                   ? at_the_end_of_time
                                   : make_timeout_time_ms(timeout_ms);

        while (true) {
            if constexpr (ThreadSafe) {
                critical_section_enter_blocking(&lock);
            }

            if (count >= CIRCULARBUFFERSIZE) {
                if (!overwriteIfFull) {
                    if constexpr (ThreadSafe) {
                        critical_section_exit(&lock);
                    }
                    if (timeout_ms == 0 || time_reached(deadline)) return false;
                    tight_loop_contents();
                    continue;
                }
                head = (head + 1) & MASK; // Advance head (overwrite oldest)
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
                critical_section_exit(&lock);
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

#endif /* ARDUINO_ARCH_RP2040 || ARDUINO_ARCH_RP2350 */

#endif /* _QUEUE_H_ */
