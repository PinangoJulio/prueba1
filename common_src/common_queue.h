#ifndef COMMON_QUEUE_H
#define COMMON_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>

//////////////////////// NON-BLOCKING QUEUE ////////////////////////

template <typename T>
class NonBlockingQueue {
private:
    std::queue<T> queue;
    std::mutex mutex;
    bool closed;

public:
    NonBlockingQueue(): closed(false) {}

    void push(T&& item) {
        std::unique_lock<std::mutex> lock(mutex);
        if (!closed) {
            queue.push(std::move(item));
        }
    }

    std::optional<T> try_pop() {
        std::unique_lock<std::mutex> lock(mutex);
        if (queue.empty()) {
            return std::nullopt;
        }
        T item = std::move(queue.front());
        queue.pop();
        return item;
    }

    void close() {
        std::unique_lock<std::mutex> lock(mutex);
        closed = true;
    }

    bool is_closed() {
        std::unique_lock<std::mutex> lock(mutex);
        return closed;
    }

    NonBlockingQueue(const NonBlockingQueue&) = delete;
    NonBlockingQueue& operator=(const NonBlockingQueue&) = delete;
};

//////////////////////// BLOCKING QUEUE (UNBOUNDED) ////////////////////////

template <typename T>
class BlockingQueue {
private:
    std::queue<T> queue;
    std::mutex mutex;
    std::condition_variable cv;
    bool closed;

public:
    BlockingQueue(): closed(false) {}

    void push(T&& item) {
        std::unique_lock<std::mutex> lock(mutex);
        if (!closed) {
            queue.push(std::move(item));
            cv.notify_one();
        }
    }

    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(mutex);

        cv.wait(lock, [this]() { return !queue.empty() || closed; });

        if (closed && queue.empty()) {
            return std::nullopt;
        }

        T item = std::move(queue.front());
        queue.pop();
        return item;
    }

    void close() {
        std::unique_lock<std::mutex> lock(mutex);
        closed = true;
        cv.notify_all();
    }

    bool is_closed() {
        std::unique_lock<std::mutex> lock(mutex);
        return closed;
    }

    BlockingQueue(const BlockingQueue&) = delete;
    BlockingQueue& operator=(const BlockingQueue&) = delete;
};

//////////////////////// BOUNDED BLOCKING QUEUE ////////////////////////

template <typename T>
class BoundedBlockingQueue {
private:
    std::queue<T> queue;
    std::mutex mutex;
    std::condition_variable cv_producers;
    std::condition_variable cv_consumers;
    size_t max_size;
    bool closed;

public:
    explicit BoundedBlockingQueue(size_t capacity): max_size(capacity), closed(false) {}

    bool push(T&& item) {
        std::unique_lock<std::mutex> lock(mutex);
        cv_producers.wait(lock, [this]() { return queue.size() < max_size || closed; });

        if (closed) {
            return false;
        }

        queue.push(std::move(item));
        cv_consumers.notify_one();
        return true;
    }

    bool try_push(T&& item) {
        std::unique_lock<std::mutex> lock(mutex);
        if (closed || queue.size() >= max_size) {
            return false;
        }
        queue.push(std::move(item));
        cv_consumers.notify_one();
        return true;
    }

    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(mutex);
        cv_consumers.wait(lock, [this]() { return !queue.empty() || closed; });

        if (closed && queue.empty()) {
            return std::nullopt;
        }

        T item = std::move(queue.front());
        queue.pop();
        cv_producers.notify_one();
        return item;
    }

    void close() {
        std::unique_lock<std::mutex> lock(mutex);
        closed = true;
        cv_producers.notify_all();
        cv_consumers.notify_all();
    }

    bool is_closed() {
        std::unique_lock<std::mutex> lock(mutex);
        return closed;
    }

    size_t size() {
        std::unique_lock<std::mutex> lock(mutex);
        return queue.size();
    }

    size_t capacity() const { return max_size; }

    BoundedBlockingQueue(const BoundedBlockingQueue&) = delete;
    BoundedBlockingQueue& operator=(const BoundedBlockingQueue&) = delete;
};

#endif  // COMMON_QUEUE_H
