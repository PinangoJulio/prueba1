#ifndef MONITOR_CLIENTS_H
#define MONITOR_CLIENTS_H

#include <algorithm>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include "../common_src/common_protocol_threads.h"

class ClientHandler;

/*
 * Monitor que protege la lista de clientes.
 *
 * RESPONSABILIDAD: Gestionar acceso thread-safe a los clientes.
 * NO tiene lógica del juego, solo operaciones sobre la colección.
 */
class MonitorClients {
private:
    std::vector<std::unique_ptr<ClientHandler>> clients;
    std::mutex mtx;
    int next_client_id;

public:
    MonitorClients(): next_client_id(0) {}

    //////////////////////// CRITICAL SECTIONS ////////////////////////

    // Agrega un nuevo cliente (thread-safe)
    void add_client(std::unique_ptr<ClientHandler> client);

    // Elimina clientes muertos (reaper)
    void remove_dead_clients();

    // Broadcast: envía evento a TODOS los clientes vivos
    void broadcast(const NitroEvent& event);

    // Obtiene el próximo ID disponible
    int get_next_id();

    // Ejecuta función sobre todos los clientes (para gameloop)
    template <typename Func>
    void for_each_client(Func func) {
        std::unique_lock<std::mutex> lock(mtx);
        for (auto& client: clients) {
            func(client.get());
        }
    }

    // Detiene TODOS los clientes
    void stop_all();

    MonitorClients(const MonitorClients&) = delete;
    MonitorClients& operator=(const MonitorClients&) = delete;
};

#endif  // MONITOR_CLIENTS_H
