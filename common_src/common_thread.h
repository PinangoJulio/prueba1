#ifndef COMMON_THREAD_H
#define COMMON_THREAD_H

#include <thread>
#include <utility>

class Thread {
private:
    std::thread thread;

public:
    Thread() = default;

    template <typename Callable, typename... Args>
    explicit Thread(Callable&& func, Args&&... args):
            thread(std::forward<Callable>(func), std::forward<Args>(args)...) {}

    void join() {
        if (thread.joinable()) {
            thread.join();
        }
    }

    Thread(Thread&& other) = default;
    Thread& operator=(Thread&& other) = default;
    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;


    ~Thread() {
        if (thread.joinable()) {
            thread.join();
        }
    }
};

#endif  // COMMON_THREAD_H
