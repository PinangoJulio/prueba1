#include "client_handler.h"

#include <iostream>
#include <utility>

ClientHandler::ClientHandler(int id, Socket skt, NonBlockingQueue<GameCommand>& game_queue):
        client_id(id),
        socket(std::move(skt)),
        protocol(socket),
        game_commands(game_queue),
        running(false) {}

void ClientHandler::start() {
    running = true;
    receiver_thread = Thread([this]() { this->receiver_loop(); });
    sender_thread = Thread([this]() { this->sender_loop(); });
}

void ClientHandler::stop() {
    running = false;
    send_queue.close();
}

//////////////////////// THREAD RECEIVER ////////////////////////

void ClientHandler::receiver_loop() {
    try {
        while (running) {
            uint8_t cmd = protocol.receive_command();

            if (cmd == CMD_ACTIVATE_NITRO) {
                GameCommand game_cmd;
                game_cmd.client_id = client_id;
                game_commands.push(std::move(game_cmd));
            }
        }
    } catch (const std::exception& e) {
        running = false;
    }
}

//////////////////////// THREAD SENDER ////////////////////////

void ClientHandler::sender_loop() {
    try {
        while (running) {
            std::optional<NitroEvent> event = send_queue.pop();

            if (!event.has_value()) {
                break;
            }

            protocol.send_nitro_event(event.value());
        }
    } catch (const std::exception& e) {
        running = false;
    }
}

//////////////////////// ENVIAR EVENTO ////////////////////////

void ClientHandler::send_event(const NitroEvent& event) { send_queue.push(NitroEvent(event)); }
