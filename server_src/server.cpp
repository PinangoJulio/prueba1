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
    running = true;

    gameloop.start();
    acceptor.start([this](Socket skt) { this->on_new_client(std::move(skt)); });
}

void Server::stop() {
    running = false;

    // Orden correcto de shutdown:
    // 1. Dejar de aceptar nuevos clientes
    acceptor.stop();

    // 2. Cerrar la queue de comandos para que el gameloop sepa que debe terminar
    game_commands.close();

    // 3. Detener el gameloop (espera a que termine su iteración actual)
    gameloop.stop();

    // 4. Detener todos los clientes (cierra sus queues)
    clients_monitor.stop_all();

    // 5. Hacer join de todos los threads de clientes y limpiar
    clients_monitor.clear();
}

void Server::wait_for_finish() {
    // El acceptor y gameloop ya se joinean en stop()
    // Los clientes ya se joinearon en clients_monitor.clear()
}

void Server::on_new_client(Socket client_socket) {
    // Verificamos si aún estamos aceptando clientes
    if (!running) {
        // El socket se cerrará automáticamente al salir de scope
        return;
    }

    int client_id = next_client_id++;

    auto handler =
            std::make_unique<ClientHandler>(client_id, std::move(client_socket), game_commands);

    handler->start();
    clients_monitor.add_client(std::move(handler));
}
