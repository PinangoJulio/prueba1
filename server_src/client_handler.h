#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <atomic>
#include <memory>

#include "../common_src/common_protocol_threads.h"
#include "../common_src/common_queue.h"
#include "../common_src/common_socket.h"
#include "../common_src/common_thread.h"

#include "car.h"

// Comando para el gameloop
struct GameCommand {
    int client_id;  // ID del cliente que envió el comando
};

class ClientHandler {
private:
    int client_id;
    Socket socket;
    ProtocolThreads protocol;
    Car car;

    // Queues
    NonBlockingQueue<GameCommand>& game_commands;  // Compartida con gameloop
    BlockingQueue<NitroEvent> send_queue;          // Queue propia para enviar

    // Threads
    Thread receiver_thread;
    Thread sender_thread;

    std::atomic<bool> running;
    std::atomic<bool> is_dead;  // Marca si el cliente está muerto

    //////////////////////// THREADS ////////////////////////

    void receiver_loop();

    void sender_loop();

public:
    ClientHandler(int id, Socket skt, NonBlockingQueue<GameCommand>& game_queue);

    void start();

    void stop();

    // Hace join de los threads (el "RIP")
    void join();

    // Verifica si el cliente está muerto (desconectado)
    bool dead() const { return is_dead; }

    void send_event(const NitroEvent& event);

    Car& get_car() { return car; }

    int get_id() const { return client_id; }

    ClientHandler(const ClientHandler&) = delete;
    ClientHandler& operator=(const ClientHandler&) = delete;
    ClientHandler(ClientHandler&&) = default;
    ClientHandler& operator=(ClientHandler&&) = default;
};

#endif  // CLIENT_HANDLER_H
