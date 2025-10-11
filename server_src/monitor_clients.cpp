#include "monitor_clients.h"

#include "../common_src/queue.h"

#include "client_handler.h"

void MonitorClients::add_client(std::unique_ptr<ClientHandler> client) {
    std::unique_lock<std::mutex> lock(mtx);
    clients.push_back(std::move(client));
}

int MonitorClients::get_next_id() {
    std::unique_lock<std::mutex> lock(mtx);
    return next_client_id++;
}

void MonitorClients::remove_dead_clients() {
    std::unique_lock<std::mutex> lock(mtx);

    // Reaper: Eliminar clientes que ya no están vivos
    clients.erase(std::remove_if(clients.begin(), clients.end(),
                                 [](const std::unique_ptr<ClientHandler>& client) {
                                     return !client->is_alive();
                                 }),
                  clients.end());
}

void MonitorClients::broadcast(const NitroEvent& event) {
    std::unique_lock<std::mutex> lock(mtx);

    for (auto& client: clients) {
        if (client->is_alive()) {
            try {
                client->send_event(event);
            } catch (const ClosedQueue&) {
                // Cliente desconectándose, ignorar
            }
        }
    }
}

void MonitorClients::stop_all() {
    std::unique_lock<std::mutex> lock(mtx);

    for (auto& client: clients) {
        client->stop();
    }

    clients.clear();
}
