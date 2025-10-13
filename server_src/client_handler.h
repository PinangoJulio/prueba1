#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <atomic>
#include <memory>

#include "../common_src/common_protocol_threads.h"
#include "../common_src/common_queue.h"
#include "../common_src/common_socket.h"
#include "../common_src/common_thread.h"

#include "car.h"

struct GameCommand {
    int client_id;
};

// Capacidad de la Queue para limitar la memoria
// PUEDE MODIFICARSE A CONVENIENCIA, se usó el 50 de forma arbitraria
constexpr size_t SEND_QUEUE_CAPACITY = 50;


class ClientHandler {
private:
    int client_id;

    Socket socket;
    ProtocolThreads protocol;

    Car car;

    // Queue compartida con el gameloop para enviar comandos
    NonBlockingQueue<GameCommand>& game_commands;

    // Queue propia para recibir eventos a enviar al cliente
    BoundedBlockingQueue<NitroEvent> send_queue;

    Thread receiver_thread;
    Thread sender_thread;

    std::atomic<bool> running;
    std::atomic<bool> dead_flag;

    //////////////////////// LOOPS DE THREADS ////////////////////////

    void receiver_loop();
    void sender_loop();

public:
    ClientHandler(int id, Socket skt, NonBlockingQueue<GameCommand>& game_queue);

    void start();

    void stop();

    void join();

    bool is_dead() const { return dead_flag.load(std::memory_order_acquire); }

    void send_event(const NitroEvent& event);

    Car& get_car() { return car; }

    int get_id() const { return client_id; }

    ClientHandler(const ClientHandler&) = delete;
    ClientHandler& operator=(const ClientHandler&) = delete;
    ClientHandler(ClientHandler&&) = default;
    ClientHandler& operator=(ClientHandler&&) = default;
};

#endif  // CLIENT_HANDLER_H
