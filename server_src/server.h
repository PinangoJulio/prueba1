#ifndef SERVER_H
#define SERVER_H

#include <memory>
#include <string>
#include <utility>

#include "../common_src/common_socket.h"
#include "../common_src/queue.h"

#include "acceptor_thread.h"
#include "gameloop_thread.h"
#include "monitor_clients.h"

/*
 * Servidor
 *
 * RESPONSABILIDAD:
 * - Orquestar los componentes (Monitor, Acceptor, GameLoop)
 * - Iniciar y detener el servidor de forma ordenada
 * - NO tiene lógica del juego ni manejo de clientes
 */
class Server {
private:
    Socket acceptor_socket;

    // Componentes (en stack, inicializados en orden correcto)
    Queue<GameCommand> game_commands;
    MonitorClients monitor;

    // Threads (en heap porque no son movibles)
    std::unique_ptr<GameLoopThread> gameloop;
    std::unique_ptr<AcceptorThread> acceptor;

public:
    explicit Server(const std::string& port):
            acceptor_socket(port.c_str()), game_commands() {  // Unbounded queue por defecto

        // Crear threads en heap
        gameloop = std::make_unique<GameLoopThread>(monitor, game_commands);
        acceptor = std::make_unique<AcceptorThread>(acceptor_socket, monitor, game_commands);
    }

    void start() {
        // Iniciar gameloop PRIMERO (así está listo para procesar)
        gameloop->start();

        // Luego aceptador
        acceptor->start();
    }

    void stop() {
        // 1. Detener acceptor (no más clientes nuevos)
        acceptor->stop();
        try {
            acceptor_socket.shutdown(2);
            acceptor_socket.close();
        } catch (...) {
            // Ignorar errores al cerrar
        }

        // 2. Detener gameloop
        gameloop->stop();

        // 3. Cerrar queue de comandos
        game_commands.close();

        // 4. Detener todos los clientes
        monitor.stop_all();
    }

    void wait_for_finish() {
        acceptor->join();
        gameloop->join();
    }

    ~Server() {
        stop();
        wait_for_finish();
    }

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
};

#endif  // SERVER_H
