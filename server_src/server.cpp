#include "server.h"

#include <iostream>
#include <memory>
#include <utility>

//////////////////////// CONSTRUCTOR ////////////////////////

Server::Server(const std::string& port):
        acceptor(port.c_str()),
        gameloop(clients_monitor, game_commands),
        running(false),
        next_client_id(0) {}

//////////////////////// CONTROL DE CICLO DE VIDA ////////////////////////

void Server::start() {
    running.store(true, std::memory_order_release);

    // Iniciamos el gameloop primero
    gameloop.start();

    // Luego el acceptor (que comenzará a agregar clientes)
    acceptor.start([this](Socket skt) { this->on_new_client(std::move(skt)); });
}

void Server::stop() {
    running.store(false, std::memory_order_release);

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
    // Todos los threads ya se joinearon en stop()
    // Este método existe por si en el futuro se necesita alguna limpieza adicional
}

//////////////////////// CALLBACKS ////////////////////////

void Server::on_new_client(Socket client_socket) {
    // Verificamos si aún estamos aceptando clientes
    if (!running.load(std::memory_order_acquire)) {
        // El socket se cerrará automáticamente al salir de scope
        return;
    }

    // Asignamos un ID único al cliente
    int client_id = next_client_id.fetch_add(1, std::memory_order_relaxed);

    // Creamos el handler del cliente
    auto handler =
            std::make_unique<ClientHandler>(client_id, std::move(client_socket), game_commands);

    // Iniciamos sus threads
    handler->start();

    // Lo agregamos al monitor
    clients_monitor.add_client(std::move(handler));
}
