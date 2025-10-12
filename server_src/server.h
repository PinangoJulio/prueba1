#ifndef SERVER_H
#define SERVER_H

#include <atomic>
#include <string>

#include "../common_src/common_queue.h"
#include "../common_src/common_socket.h"

#include "acceptor.h"
#include "client_handler.h"
#include "clients_monitor.h"
#include "gameloop.h"

class Server {
private:
    Acceptor acceptor;
    ClientsMonitor clients_monitor;
    NonBlockingQueue<GameCommand> game_commands;
    GameLoop gameloop;

    std::atomic<bool> running;
    int next_client_id;

    void on_new_client(Socket client_socket);

public:
    explicit Server(const std::string& port);

    void start();

    void stop();

    void wait_for_finish();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
};

#endif  // SERVER_H
