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

//////////////////////// CONFIGURACIÓN ////////////////////////

constexpr size_t GAME_COMMANDS_QUEUE_CAPACITY = 100;

//////////////////////// SERVER ////////////////////////

class Server {
private:
    //////////////////////// COMPONENTES ////////////////////////

    Acceptor acceptor;
    ClientsMonitor clients_monitor;
    NonBlockingQueue<GameCommand> game_commands;
    GameLoop gameloop;

    //////////////////////// ESTADO ////////////////////////

    std::atomic<bool> running;
    std::atomic<int> next_client_id;

    //////////////////////// CALLBACKS ////////////////////////

    // Callback para cuando un nuevo cliente se conecta
    void on_new_client(Socket client_socket);

public:
    //////////////////////// CONSTRUCTOR ////////////////////////

    explicit Server(const std::string& port);

    //////////////////////// CONTROL DE CICLO DE VIDA ////////////////////////

    // Inicia el servidor (acceptor y gameloop)
    void start();

    // Detiene el servidor ordenadamente
    void stop();

    // Espera a que todos los threads terminen
    void wait_for_finish();

    //////////////////////// MOVIMIENTO Y COPIA ////////////////////////

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
};

#endif  // SERVER_H
