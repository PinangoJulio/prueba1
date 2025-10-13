#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <atomic>
#include <memory>

#include "../common_src/common_protocol_threads.h"
#include "../common_src/common_queue.h"
#include "../common_src/common_socket.h"
#include "../common_src/common_thread.h"

#include "car.h"

//////////////////////// COMANDOS ////////////////////////

// Comando enviado al gameloop cuando un cliente activa nitro
struct GameCommand {
    int client_id;
};

//////////////////////// CONFIGURACIÓN ////////////////////////

constexpr size_t SEND_QUEUE_CAPACITY = 50;

//////////////////////// CLIENT HANDLER ////////////////////////

class ClientHandler {
private:
    //////////////////////// IDENTIFICACIÓN ////////////////////////

    int client_id;

    //////////////////////// COMUNICACIÓN ////////////////////////

    Socket socket;
    ProtocolThreads protocol;

    //////////////////////// ESTADO DEL JUEGO ////////////////////////

    Car car;

    //////////////////////// QUEUES ////////////////////////

    // Queue compartida con el gameloop para enviar comandos
    NonBlockingQueue<GameCommand>& game_commands;

    // Queue propia para recibir eventos a enviar al cliente
    BoundedBlockingQueue<NitroEvent> send_queue;

    //////////////////////// THREADS ////////////////////////

    Thread receiver_thread;
    Thread sender_thread;

    //////////////////////// ESTADO DE CONTROL ////////////////////////

    std::atomic<bool> running;
    std::atomic<bool> dead_flag;

    //////////////////////// LOOPS DE THREADS ////////////////////////

    // Recibe comandos del cliente y los encola para el gameloop
    void receiver_loop();

    // Envía eventos al cliente desde la queue
    void sender_loop();

public:
    //////////////////////// CONSTRUCTOR ////////////////////////

    ClientHandler(int id, Socket skt, NonBlockingQueue<GameCommand>& game_queue);

    //////////////////////// CONTROL DE CICLO DE VIDA ////////////////////////

    // Inicia los threads receptor y emisor
    void start();

    // Detiene los threads y cierra la queue de envío
    void stop();

    // Espera a que los threads terminen (RIP)
    void join();

    //////////////////////// ESTADO ////////////////////////

    // Verifica si el cliente está desconectado
    bool is_dead() const { return dead_flag.load(std::memory_order_acquire); }

    //////////////////////// COMUNICACIÓN ////////////////////////

    // Encola un evento para enviar al cliente
    void send_event(const NitroEvent& event);

    //////////////////////// ACCESO AL ESTADO DEL JUEGO ////////////////////////

    Car& get_car() { return car; }

    int get_id() const { return client_id; }

    //////////////////////// MOVIMIENTO Y COPIA ////////////////////////

    ClientHandler(const ClientHandler&) = delete;
    ClientHandler& operator=(const ClientHandler&) = delete;
    ClientHandler(ClientHandler&&) = default;
    ClientHandler& operator=(ClientHandler&&) = default;
};

#endif  // CLIENT_HANDLER_H
