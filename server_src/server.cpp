#include "server.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#define GAMELOOP_SLEEP_MS 250  // 4 loops/segundo = 250ms

Server::Server(const std::string& port):
        acceptor(port.c_str()), running(false), next_client_id(0) {}

void Server::start() {
    running = true;

    gameloop_thread = Thread([this]() { this->gameloop(); });
    acceptor.start([this](Socket skt) { this->on_new_client(std::move(skt)); });
}

void Server::stop() {
    running = false;

    acceptor.stop();

    // Esperamos a que el gameloop termine su iteración actual
    gameloop_thread.join();

    game_commands.close();
    clients_monitor.stop_all();
    clients_monitor.clear();
}

void Server::wait_for_finish() {
    // El acceptor ya se joinea en stop()
}

//////////////////////// CLIENT MANAGEMENT ////////////////////////

void Server::on_new_client(Socket client_socket) {
    int client_id = next_client_id++;

    auto handler =
            std::make_unique<ClientHandler>(client_id, std::move(client_socket), game_commands);

    handler->start();
    clients_monitor.add_client(std::move(handler));
}

//////////////////////// GAMELOOP THREAD ////////////////////////

void Server::gameloop() {
    while (running) {
        process_commands();
        simulate_world();
        std::this_thread::sleep_for(std::chrono::milliseconds(GAMELOOP_SLEEP_MS));
    }
}

void Server::process_commands() {
    while (running) {
        std::optional<GameCommand> cmd = game_commands.try_pop();

        if (!cmd.has_value()) {
            break;
        }

        bool activated = false;
        clients_monitor.apply_to_client(cmd->client_id, [&](ClientHandler& client) {
            activated = client.get_car().activate_nitro();
        });

        if (activated) {
            uint16_t cars_with_nitro = count_cars_with_nitro();
            NitroEvent event(cars_with_nitro, EVENT_NITRO_ACTIVATED);

            std::cout << "A car hit the nitro!" << std::endl;
            broadcast_event(event);
        }
    }
}

void Server::simulate_world() {
    // Identificamos qué autos van a expirar ANTES de actualizar nada
    std::vector<int> about_to_expire;
    std::vector<int> all_clients;

    clients_monitor.apply_to_all([&](ClientHandler& client) {
        all_clients.push_back(client.get_id());
        if (client.get_car().will_expire()) {
            about_to_expire.push_back(client.get_id());
        }
    });

    // Actualizamos todos los autos que NO van a expirar
    for (int client_id: all_clients) {
        // Verificamos si está en la lista de los que van a expirar usando std::any_of
        bool will_expire = std::any_of(about_to_expire.begin(), about_to_expire.end(),
                                       [client_id](int id) { return id == client_id; });

        if (!will_expire) {
            clients_monitor.apply_to_client(
                    client_id, [&](ClientHandler& client) { client.get_car().update(); });
        }
    }

    // Para cada auto que va a expirar, lo actualizamos Y enviamos el evento
    for (int client_id: about_to_expire) {
        clients_monitor.apply_to_client(client_id,
                                        [&](ClientHandler& client) { client.get_car().update(); });

        uint16_t cars_with_nitro = count_cars_with_nitro();
        NitroEvent event(cars_with_nitro, EVENT_NITRO_EXPIRED);

        std::cout << "A car is out of juice." << std::endl;
        broadcast_event(event);
    }
}

void Server::broadcast_event(const NitroEvent& event) {
    clients_monitor.apply_to_all([&](ClientHandler& client) { client.send_event(event); });
}

uint16_t Server::count_cars_with_nitro() {
    uint16_t count = 0;
    clients_monitor.apply_to_all([&](ClientHandler& client) {
        if (client.get_car().has_nitro()) {
            count++;
        }
    });
    return count;
}
