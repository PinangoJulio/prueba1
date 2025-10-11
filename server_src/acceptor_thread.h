#ifndef ACCEPTOR_THREAD_H
#define ACCEPTOR_THREAD_H

#include <memory>
#include <utility>

#include "../common_src/common_socket.h"
#include "../common_src/queue.h"
#include "../common_src/thread.h"

#include "client_handler.h"
#include "monitor_clients.h"

/*
 * Thread Aceptador
 *
 * RESPONSABILIDAD:
 * - Aceptar conexiones entrantes
 * - Crear ClientHandlers
 * - Agregarlos al monitor
 */
class AcceptorThread: public Thread {
private:
    Socket& acceptor_socket;
    MonitorClients& monitor;
    Queue<GameCommand>& game_commands;

public:
    AcceptorThread(Socket& skt, MonitorClients& mon, Queue<GameCommand>& queue):
            acceptor_socket(skt), monitor(mon), game_commands(queue) {}

    void run() override {
        while (should_keep_running()) {
            try {
                Socket client_socket = acceptor_socket.accept();

                int client_id = monitor.get_next_id();

                auto handler = std::make_unique<ClientHandler>(client_id, std::move(client_socket),
                                                               game_commands);

                handler->start();
                monitor.add_client(std::move(handler));

            } catch (const std::exception&) {
                // Socket cerrado o error, salir
                break;
            }
        }
    }
};

#endif  // ACCEPTOR_THREAD_H
