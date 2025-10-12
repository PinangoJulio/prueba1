#ifndef SERVER_H
#define SERVER_H

#include <atomic>
#include <string>

#include "../common_src/common_queue.h"
#include "../common_src/common_socket.h"
#include "../common_src/common_thread.h"

#include "client_handler.h"
#include "clients_monitor.h"

class Server {
private:
    Socket acceptor_socket;
    std::atomic<bool> running;
    int next_client_id;

    NonBlockingQueue<GameCommand> game_commands;
    ClientsMonitor clients_monitor;

    Thread acceptor_thread;
    Thread gameloop_thread;

    //////////////////////// ACCEPTOR THREAD ////////////////////////

    void acceptor_loop();

    void add_client(Socket client_socket);

    //////////////////////// GAMELOOP THREAD ////////////////////////

    void gameloop();

    void process_commands();

    void simulate_world();

    void broadcast_event(const NitroEvent& event);

    uint16_t count_cars_with_nitro();

public:
    explicit Server(const std::string& port);

    void start();

    void stop();

    void wait_for_finish();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
};

#endif  // SERVER_H
