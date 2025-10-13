#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <atomic>
#include <functional>

#include "../common_src/common_socket.h"
#include "../common_src/common_thread.h"

//////////////////////// ACCEPTOR ////////////////////////

class Acceptor {
private:
    Socket acceptor_socket;
    Thread acceptor_thread;
    std::atomic<bool> running;
    std::function<void(Socket)> on_new_client;

    //////////////////////// LOOP DEL ACCEPTOR ////////////////////////
    void acceptor_loop();

public:
    explicit Acceptor(const char* port);

    void start(std::function<void(Socket)> callback);

    void stop();

    Acceptor(const Acceptor&) = delete;
    Acceptor& operator=(const Acceptor&) = delete;
};

#endif  // ACCEPTOR_H
