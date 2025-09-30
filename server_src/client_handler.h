#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <memory>
#include "../common_src/common_socket.h"
#include "../common_src/common_protocol_threads.h"
#include "../common_src/common_queue.h"
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
    
    bool running;

    //////////////////////// THREADS ////////////////////////
    
    // Thread receiver: lee del socket y pushea comandos al gameloop
    void receiver_loop();
    
    // Thread sender: lee de su queue y envía por socket
    void sender_loop();

public:
    ClientHandler(int id, Socket skt, NonBlockingQueue<GameCommand>& game_queue);

    // Iniciar threads
    void start();

    // Detener threads
    void stop();

    // Enviar evento a este cliente (llamado por gameloop desde broadcast)
    void send_event(const NitroEvent& event);

    // Obtener referencia al auto de este cliente
    Car& get_car() { return car; }
    
    int get_id() const { return client_id; }

    ClientHandler(const ClientHandler&) = delete;
    ClientHandler& operator=(const ClientHandler&) = delete;
    ClientHandler(ClientHandler&&) = default;
    ClientHandler& operator=(ClientHandler&&) = default;
};

#endif  // CLIENT_HANDLER_H
