#include "client_handler.h"

#include <iostream>
#include <utility>

ClientHandler::ClientHandler(int id, Socket skt, NonBlockingQueue<GameCommand>& game_queue):
        client_id(id),
        socket(std::move(skt)),
        protocol(socket),
        game_commands(game_queue),
        send_queue(SEND_QUEUE_CAPACITY),
        running(false),
        dead_flag(false) {}


void ClientHandler::start() {
    running.store(true, std::memory_order_release);
    receiver_thread = Thread([this]() { this->receiver_loop(); });
    sender_thread = Thread([this]() { this->sender_loop(); });
}

void ClientHandler::stop() {
    running.store(false, std::memory_order_release);
    send_queue.close();
}

void ClientHandler::join() {
    receiver_thread.join();
    sender_thread.join();
}

//////////////////////// THREAD RECEIVER ////////////////////////

void ClientHandler::receiver_loop() {
    try {
        while (running.load(std::memory_order_acquire)) {
            uint8_t cmd = protocol.receive_command();

            if (cmd == CMD_ACTIVATE_NITRO) {
                GameCommand game_cmd{client_id};
                game_commands.push(std::move(game_cmd));
            }
        }
    } catch (const std::exception& e) {
        // o el Cliente está desconectado o es un error de la red
        running.store(false, std::memory_order_release);
        dead_flag.store(true, std::memory_order_release);
        send_queue.close();
    }
}

//////////////////////// THREAD SENDER ////////////////////////

void ClientHandler::sender_loop() {
    try {
        while (running.load(std::memory_order_acquire)) {
            std::optional<NitroEvent> event = send_queue.pop();

            if (!event.has_value()) {
                // La Queue debe estar cerrada en este punto
                break;
            }

            protocol.send_nitro_event(event.value());
        }
    } catch (const std::exception& e) {
        // Error al enviar, o el Cliente está desconectado o es un error de la red
        running.store(false, std::memory_order_release);
        dead_flag.store(true, std::memory_order_release);
    }
}

//////////////////////// COMUNICACIÓN ////////////////////////

void ClientHandler::send_event(const NitroEvent& event) { send_queue.push(NitroEvent(event)); }
