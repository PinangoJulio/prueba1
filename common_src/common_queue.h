#ifndef COMMON_QUEUE_H
#define COMMON_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>

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

#endif  // COMMON_QUEUE_H
