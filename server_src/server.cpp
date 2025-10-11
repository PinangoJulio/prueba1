#include "server.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <utility>

#define GAMELOOP_SLEEP_MS 250  // 4 loops/segundo = 250ms. SE PUEDE MODIFICAR A CONVENIENCIA

Server::Server(const std::string& port):
        acceptor_socket(port.c_str()), running(false), next_client_id(0) {}

void Server::start() {
    running = true;

    gameloop_thread = Thread([this]() { this->gameloop(); });
    acceptor_thread = Thread([this]() { this->acceptor_loop(); });
}

void Server::stop() {
    running = false;

    try {
        acceptor_socket.shutdown(2);
    } catch (...) {
        // ESto sehace para poder ignorar errores al cerrar
    }

    game_commands.close();

    {
        std::unique_lock<std::mutex> lock(clients_mutex);
        for (auto& client: clients) {
            client->stop();
        }
        clients.clear();
    }
}

void Server::wait_for_finish() {
    // Los threads se joinean automáticamente en sus destructores
}

//////////////////////// ACCEPTOR THREAD ////////////////////////

void Server::acceptor_loop() {
    while (running) {
        try {
            Socket client_socket = acceptor_socket.accept();
            add_client(std::move(client_socket));
        } catch (const std::exception& e) {
            break;
        }
    }
}

void Server::add_client(Socket client_socket) {
    std::unique_lock<std::mutex> lock(clients_mutex);

    int client_id = next_client_id++;

    auto handler =
            std::make_unique<ClientHandler>(client_id, std::move(client_socket), game_commands);

    handler->start();
    clients.push_back(std::move(handler));
}

//////////////////////// GAMELOOP THREAD ////////////////////////

void Server::gameloop() {
    while (running) {
        process_commands();
        simulate_world();
        for (int i = 0; i < 25 && running; i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

void Server::process_commands() {
    while (running) {
        std::optional<GameCommand> cmd = game_commands.try_pop();

        if (!cmd.has_value()) {
            break;
        }

        std::unique_lock<std::mutex> lock(clients_mutex);

        auto client_it = std::find_if(clients.begin(), clients.end(), [&](const auto& client) {
            return client->get_id() == cmd->client_id;
        });

        if (client_it != clients.end()) {
            bool activated = (*client_it)->get_car().activate_nitro();

            if (activated) {
                uint16_t cars_with_nitro = count_cars_with_nitro();
                NitroEvent event(cars_with_nitro, EVENT_NITRO_ACTIVATED);

                std::cout << "A car hit the nitro!" << std::endl;
                broadcast_event(event);
            }
        }
    }
}

void Server::simulate_world() {
    std::unique_lock<std::mutex> lock(clients_mutex);

    for (auto& client: clients) {
        bool expired = client->get_car().update();

        if (expired) {
            uint16_t cars_with_nitro = count_cars_with_nitro();
            NitroEvent event(cars_with_nitro, EVENT_NITRO_EXPIRED);

            std::cout << "A car is out of juice." << std::endl;
            broadcast_event(event);
        }
    }
}

void Server::broadcast_event(const NitroEvent& event) {
    for (auto& client: clients) {
        client->send_event(event);
    }
}

uint16_t Server::count_cars_with_nitro() {
    uint16_t count = 0;
    for (auto& client: clients) {
        if (client->get_car().has_nitro()) {
            count++;
        }
    }
    return count;
}
