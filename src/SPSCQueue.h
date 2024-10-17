#ifndef SPSCQUEUE_H
#define SPSCQUEUE_H

#include <vector>
#include <atomic>
#include <cassert>

const size_t QUEUE_CAPACITY = 1 << 16; // 必须是2的幂

template <typename T>
class SPSCQueue {
public:
    SPSCQueue(size_t capacity = QUEUE_CAPACITY) : head_(0), tail_(0), buffer_(capacity) {
        assert((capacity & (capacity -1)) == 0 && "Queue capacity must be a power of 2");
    }

    bool push(const T& item) {
        size_t head = head_.load(std::memory_order_relaxed);
        size_t next_head = (head + 1) & (buffer_.size() - 1);
        if (next_head == tail_.load(std::memory_order_acquire)) {
            // 队列满
            return false;
        }
        buffer_[head] = item;
        head_.store(next_head, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        size_t tail = tail_.load(std::memory_order_relaxed);
        if (tail == head_.load(std::memory_order_acquire)) {
            // 队列空
            return false;
        }
        item = buffer_[tail];
        tail_.store((tail + 1) & (buffer_.size() -1), std::memory_order_release);
        return true;
    }

private:
    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;
    std::vector<T> buffer_;
};

#endif // SPSCQUEUE_H

