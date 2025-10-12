#ifndef CLIENTS_MONITOR_H
#define CLIENTS_MONITOR_H

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "client_handler.h"

class ClientsMonitor {
private:
    std::vector<std::unique_ptr<ClientHandler>> clients;
    std::mutex mutex;

public:
    ClientsMonitor() = default;

    // Agrega un nuevo cliente al monitor
    void add_client(std::unique_ptr<ClientHandler> client) {
        std::unique_lock<std::mutex> lock(mutex);
        clients.push_back(std::move(client));
    }

    // Aplica una función a todos los clientes (para broadcast)
    void apply_to_all(const std::function<void(ClientHandler&)>& func) {
        std::unique_lock<std::mutex> lock(mutex);
        for (auto& client : clients) {
            func(*client);
        }
    }

    // Busca un cliente por ID y aplica una función si lo encuentra
    bool apply_to_client(int client_id, const std::function<void(ClientHandler&)>& func) {
        std::unique_lock<std::mutex> lock(mutex);
        for (auto& client : clients) {
            if (client->get_id() == client_id) {
                func(*client);
                return true;
            }
        }
        return false;
    }

    // Detiene todos los clientes
    void stop_all() {
        std::unique_lock<std::mutex> lock(mutex);
        for (auto& client : clients) {
            client->stop();
        }
    }

    // Limpia todos los clientes
    void clear() {
        std::unique_lock<std::mutex> lock(mutex);
        clients.clear();
    }

    ClientsMonitor(const ClientsMonitor&) = delete;
    ClientsMonitor& operator=(const ClientsMonitor&) = delete;
};

#endif  // CLIENTS_MONITOR_H
