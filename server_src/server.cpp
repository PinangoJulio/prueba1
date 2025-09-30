#include "server.h"
#include <iostream>
#include <chrono>
#include <thread>

#define GAMELOOP_SLEEP_MS 250  // 4 loops/segundo = 250ms

Server::Server(const std::string& port):
        acceptor_socket(port.c_str()),
        running(false),
        next_client_id(0) {}

void Server::start() {
    running = true;
    
    // Iniciar thread acceptor
    acceptor_thread = Thread([this]() { this->acceptor_loop(); });
    
    // Iniciar thread gameloop
    gameloop_thread = Thread([this]() { this->gameloop(); });
}

void Server::stop() {
    running = false;
    
    // Cerrar queue de comandos
    game_commands.close();
    
    // Detener todos los clientes
    std::unique_lock<std::mutex> lock(clients_mutex);
    for (auto& client : clients) {
        client->stop();
    }
    clients.clear();
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
            if (running) {
                std::cerr << "Acceptor error: " << e.what() << std::endl;
            }
            break;
        }
    }
}

void Server::add_client(Socket client_socket) {
    std::unique_lock<std::mutex> lock(clients_mutex);
    
    int client_id = next_client_id++;
    
    auto handler = std::make_unique<ClientHandler>(
        client_id,
        std::move(client_socket),
        game_commands
    );
    
    handler->start();
    clients.push_back(std::move(handler));
}

//////////////////////// GAMELOOP THREAD ////////////////////////

void Server::gameloop() {
    while (running) {
        // Paso 1: Procesar comandos pendientes
        process_commands();
        
        // Paso 2: Simular mundo (expirar nitros)
        simulate_world();
        
        // Paso 4: Sleep (ÚNICO lugar con sleep en el servidor)
        std::this_thread::sleep_for(std::chrono::milliseconds(GAMELOOP_SLEEP_MS));
    }
}

void Server::process_commands() {
    // Leer TODOS los comandos pendientes (non-blocking)
    while (true) {
        std::optional<GameCommand> cmd = game_commands.try_pop();
        
        if (!cmd.has_value()) {
            break;  // No hay más comandos
        }
        
        // Buscar el cliente y activar su nitro
        std::unique_lock<std::mutex> lock(clients_mutex);
        
        for (auto& client : clients) {
            if (client->get_id() == cmd->client_id) {
                bool activated = client->get_car().activate_nitro();
                
                if (activated) {
                    // Paso 3: Broadcast
                    uint16_t cars_with_nitro = count_cars_with_nitro();
                    NitroEvent event(cars_with_nitro, EVENT_NITRO_ACTIVATED);
                    
                    std::cout << "A car hit the nitro!" << std::endl;
                    broadcast_event(event);
                }
                // Si no se activó, se ignora (ya tenía nitro activo)
                
                break;
            }
        }
    }
}

void Server::simulate_world() {
    std::unique_lock<std::mutex> lock(clients_mutex);
    
    for (auto& client : clients) {
        bool expired = client->get_car().update();
        
        if (expired) {
            // Paso 3: Broadcast
            uint16_t cars_with_nitro = count_cars_with_nitro();
            NitroEvent event(cars_with_nitro, EVENT_NITRO_EXPIRED);
            
            std::cout << "A car is out of juice." << std::endl;
            broadcast_event(event);
        }
    }
}

void Server::broadcast_event(const NitroEvent& event) {
    // Ya debe estar lockeado clients_mutex cuando se llama
    for (auto& client : clients) {
        client->send_event(event);
    }
}

uint16_t Server::count_cars_with_nitro() {
    // Ya debe estar lockeado clients_mutex cuando se llama
    uint16_t count = 0;
    for (auto& client : clients) {
        if (client->get_car().has_nitro()) {
            count++;
        }
    }
    return count;
}
