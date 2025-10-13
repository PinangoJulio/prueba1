#ifndef CLIENTS_MONITOR_H
#define CLIENTS_MONITOR_H

#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include "client_handler.h"

//////////////////////// CLIENTS MONITOR ////////////////////////

class ClientsMonitor {
private:
    std::vector<std::unique_ptr<ClientHandler>> clients;
    std::mutex mutex;

public:
    ClientsMonitor() = default;

    //////////////////////// GESTIÓN DE CLIENTES ////////////////////////

    // Agrega un nuevo cliente al monitor (Critical Section)
    void add_client(std::unique_ptr<ClientHandler> client) {
        std::unique_lock<std::mutex> lock(mutex);
        clients.push_back(std::move(client));
    }

    // Limpia los clientes muertos (hace join y los remueve) (Critical Section)
    void remove_dead_clients() {
        std::unique_lock<std::mutex> lock(mutex);

        // Primero hacemos join de los threads muertos
        for (auto& client: clients) {
            if (client->is_dead()) {
                client->join();
            }
        }

        // Removemos los clientes muertos del vector
        clients.erase(std::remove_if(clients.begin(), clients.end(),
                                     [](const std::unique_ptr<ClientHandler>& client) {
                                         return client->is_dead();
                                     }),
                      clients.end());
    }

    //////////////////////// OPERACIONES EN CLIENTES ////////////////////////

    // Aplica una función a todos los clientes VIVOS (Critical Section para broadcast)
    void apply_to_all(const std::function<void(ClientHandler&)>& func) {
        std::unique_lock<std::mutex> lock(mutex);
        for (auto& client: clients) {
            if (!client->is_dead()) {
                func(*client);
            }
        }
    }

    // Busca un cliente por ID y aplica una función si lo encuentra y está vivo
    // (Critical Section)
    bool apply_to_client(int client_id, const std::function<void(ClientHandler&)>& func) {
        std::unique_lock<std::mutex> lock(mutex);
        auto it = std::find_if(clients.begin(), clients.end(),
                               [client_id](const std::unique_ptr<ClientHandler>& client) {
                                   return client->get_id() == client_id;
                               });

        if (it != clients.end() && !(*it)->is_dead()) {
            func(*(*it));
            return true;
        }
        return false;
    }

    //////////////////////// SHUTDOWN ////////////////////////

    // Detiene todos los clientes (Critical Section)
    void stop_all() {
        std::unique_lock<std::mutex> lock(mutex);
        for (auto& client: clients) {
            client->stop();
        }
    }

    // Limpia todos los clientes (hace join de todos) (Critical Section)
    void clear() {
        std::unique_lock<std::mutex> lock(mutex);
        for (auto& client: clients) {
            client->join();
        }
        clients.clear();
    }

    //////////////////////// MOVIMIENTO Y COPIA ////////////////////////

    ClientsMonitor(const ClientsMonitor&) = delete;
    ClientsMonitor& operator=(const ClientsMonitor&) = delete;
};

#endif  // CLIENTS_MONITOR_H
