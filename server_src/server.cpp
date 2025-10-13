#include "server.h"

#include <iostream>
#include <memory>
#include <utility>

Server::Server(const std::string& port):
        acceptor(port.c_str()),
        gameloop(clients_monitor, game_commands),
        running(false),
        next_client_id(0) {}


void Server::start() {
    running.store(true, std::memory_order_release);

    gameloop.start();

    acceptor.start([this](Socket skt) { this->on_new_client(std::move(skt)); });
}

void Server::stop() {
    running.store(false, std::memory_order_release);

    // EL orden del shutdown para el server es el siguiente:

    // Dejar de aceptar a nuevos clientes
    acceptor.stop();

    // Cerrar la queue de comandos para que el gameloop sepa que tiene que terminar
    game_commands.close();

    // Frenar el gameloop (pero espera a que se termine su iteración actual)
    gameloop.stop();

    // Detener a todos los clientes
    clients_monitor.stop_all();

    // 5. Hacer join de todos los threads de clientes y limpiar
    clients_monitor.clear();
}


void Server::on_new_client(Socket client_socket) {
    if (!running.load(std::memory_order_acquire)) {
        return;
    }

    int client_id = next_client_id.fetch_add(1, std::memory_order_relaxed);

    auto handler =
            std::make_unique<ClientHandler>(client_id, std::move(client_socket), game_commands);

    handler->start();
    clients_monitor.add_client(std::move(handler));
}
