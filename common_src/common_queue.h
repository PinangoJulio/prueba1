#ifndef COMMON_QUEUE_H
#define COMMON_QUEUE_H

#include <mutex>
#include <queue>
#include <optional>
#include <condition_variable>

// Queue NO bloqueante (para GameLoop)
template<typename T>
class NonBlockingQueue {
private:
    std::queue<T> queue;
    std::mutex mutex;
    bool closed;

public:
    NonBlockingQueue(): closed(false) {}

    // Push: agregar elemento (no bloquea)
    void push(T&& item) {
        std::unique_lock<std::mutex> lock(mutex);
        if (!closed) {
            queue.push(std::move(item));
        }
    }

    // Try pop: intenta sacar elemento (no bloquea si está vacía)
    std::optional<T> try_pop() {
        std::unique_lock<std::mutex> lock(mutex);
        if (queue.empty()) {
            return std::nullopt;
        }
        T item = std::move(queue.front());
        queue.pop();
        return item;
    }

    // Cerrar la queue
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

// Queue bloqueante (para Senders)
template<typename T>
class BlockingQueue {
private:
    std::queue<T> queue;
    std::mutex mutex;
    std::condition_variable cv;
    bool closed;

public:
    BlockingQueue(): closed(false) {}

    // Push: agregar elemento y notificar
    void push(T&& item) {
        std::unique_lock<std::mutex> lock(mutex);
        if (!closed) {
            queue.push(std::move(item));
            cv.notify_one();
        }
    }

    // Pop: saca elemento (bloquea si está vacía)
    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(mutex);
        
        // Esperar hasta que haya elementos o se cierre
        cv.wait(lock, [this]() { return !queue.empty() || closed; });
        
        if (closed && queue.empty()) {
            return std::nullopt;
        }
        
        T item = std::move(queue.front());
        queue.pop();
        return item;
    }

    // Cerrar la queue
    void close() {
        std::unique_lock<std::mutex> lock(mutex);
        closed = true;
        cv.notify_all();  // Despertar a todos los threads esperando
    }

    bool is_closed() {
        std::unique_lock<std::mutex> lock(mutex);
        return closed;
    }

    BlockingQueue(const BlockingQueue&) = delete;
    BlockingQueue& operator=(const BlockingQueue&) = delete;
};

#endif  // COMMON_QUEUE_H