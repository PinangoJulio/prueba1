#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include "../common_src/common_socket.h"
#include "../common_src/common_queue.h"
#include "../common_src/common_thread.h"
#include "client_handler.h"

class Server {
private:
    Socket acceptor_socket;
    std::atomic<bool> running;
    int next_client_id;
    
    // Queue compartida para comandos del gameloop
    NonBlockingQueue<GameCommand> game_commands;
    
    // Lista de clientes (protegida con mutex)
    std::vector<std::unique_ptr<ClientHandler>> clients;
    std::mutex clients_mutex;
    
    // Threads
    Thread acceptor_thread;
    Thread gameloop_thread;

    //////////////////////// ACCEPTOR THREAD ////////////////////////
    
    // Loop del acceptor: acepta nuevos clientes
    void acceptor_loop();
    
    // Agrega un nuevo cliente a la lista (thread-safe)
    void add_client(Socket client_socket);

    //////////////////////// GAMELOOP THREAD ////////////////////////
    
    // Loop principal del juego: 4 iteraciones por segundo
    void gameloop();
    
    // Paso 1: Leer y ejecutar comandos pendientes
    void process_commands();
    
    // Paso 2: Simular iteración del mundo (expirar nitros)
    void simulate_world();
    
    // Paso 3: Broadcast de eventos a todos los clientes
    void broadcast_event(const NitroEvent& event);
    
    // Obtener cantidad de autos con nitro activo
    uint16_t count_cars_with_nitro();

public:
    explicit Server(const std::string& port);

    // Iniciar servidor (lanza threads)
    void start();

    // Detener servidor (llamado cuando se lee 'q')
    void stop();

    // Esperar a que terminen todos los threads
    void wait_for_finish();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
};

#endif  // SERVER_H
