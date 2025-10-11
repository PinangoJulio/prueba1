#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <memory>
#include <utility>

#include "../common_src/common_protocol_threads.h"
#include "../common_src/common_socket.h"
#include "../common_src/queue.h"
#include "../common_src/thread.h"

#include "car.h"

// Comando para el gameloop
struct GameCommand {
    int client_id;
};

//////////////////////// RECEIVER THREAD ////////////////////////

class ReceiverThread: public Thread {
private:
    int client_id;
    ProtocolThreads& protocol;
    Queue<GameCommand>& game_commands;

public:
    ReceiverThread(int id, ProtocolThreads& proto, Queue<GameCommand>& queue):
            client_id(id), protocol(proto), game_commands(queue) {}

    void run() override {
        try {
            while (should_keep_running()) {
                uint8_t cmd = protocol.receive_command();

                if (cmd == CMD_ACTIVATE_NITRO) {
                    GameCommand game_cmd;
                    game_cmd.client_id = client_id;

                    try {
                        game_commands.push(game_cmd);
                    } catch (const ClosedQueue&) {
                        break;  // Queue cerrada, salir
                    }
                }
            }
        } catch (const std::exception&) {
            // Cliente desconectado o socket cerrado
        }
    }
};

//////////////////////// SENDER THREAD ////////////////////////

class SenderThread: public Thread {
private:
    ProtocolThreads& protocol;
    Queue<NitroEvent> send_queue;

public:
    explicit SenderThread(ProtocolThreads& proto):
            protocol(proto), send_queue(100) {}  // Bounded queue

    void run() override {
        try {
            while (should_keep_running()) {
                NitroEvent event = send_queue.pop();
                protocol.send_nitro_event(event);
            }
        } catch (const ClosedQueue&) {
            // Queue cerrada, terminar
        } catch (const std::exception&) {
            // Error de socket
        }
    }

    void send_event(const NitroEvent& event) {
        try {
            send_queue.push(event);
        } catch (const ClosedQueue&) {
            // Ya estamos cerrando
        }
    }

    void close_queue() { send_queue.close(); }
};

//////////////////////// CLIENT HANDLER ////////////////////////

class ClientHandler {
private:
    int client_id;
    Socket socket;
    ProtocolThreads protocol;
    Car car;

    // Threads (en heap porque no son movibles)
    std::unique_ptr<ReceiverThread> receiver;
    std::unique_ptr<SenderThread> sender;

public:
    ClientHandler(int id, Socket skt, Queue<GameCommand>& game_queue):
            client_id(id), socket(std::move(skt)), protocol(socket) {

        // Crear threads en heap
        receiver = std::make_unique<ReceiverThread>(id, protocol, game_queue);
        sender = std::make_unique<SenderThread>(protocol);
    }

    void start() {
        receiver->start();
        sender->start();
    }

    void stop() {
        receiver->stop();
        sender->stop();
        sender->close_queue();

        // Cerrar socket para desbloquear receive_command()
        try {
            socket.shutdown(2);
        } catch (...) {
            // Ignorar errores
        }
    }

    void join() {
        if (receiver->is_alive()) {
            receiver->join();
        }
        if (sender->is_alive()) {
            sender->join();
        }
    }

    bool is_alive() const { return receiver->is_alive() || sender->is_alive(); }

    void send_event(const NitroEvent& event) { sender->send_event(event); }

    Car& get_car() { return car; }
    int get_id() const { return client_id; }

    ~ClientHandler() {
        stop();
        join();
    }

    ClientHandler(const ClientHandler&) = delete;
    ClientHandler& operator=(const ClientHandler&) = delete;
    ClientHandler(ClientHandler&&) = default;
    ClientHandler& operator=(ClientHandler&&) = default;
};

#endif  // CLIENT_HANDLER_H
