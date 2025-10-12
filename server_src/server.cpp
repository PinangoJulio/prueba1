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

    acceptor.stop();
    gameloop.stop();

    game_commands.close();
    clients_monitor.stop_all();
    clients_monitor.clear();
}

void Server::wait_for_finish() {
    // El acceptor y gameloop ya se joinean en stop()
}

void Server::on_new_client(Socket client_socket) {
    int client_id = next_client_id++;

    auto handler =
            std::make_unique<ClientHandler>(client_id, std::move(client_socket), game_commands);

    handler->start();
    clients_monitor.add_client(std::move(handler));
}
