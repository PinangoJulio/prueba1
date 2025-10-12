#include "client_handler.h"

#include <iostream>
#include <utility>

ClientHandler::ClientHandler(int id, Socket skt, NonBlockingQueue<GameCommand>& game_queue):
        client_id(id),
        socket(std::move(skt)),
        protocol(socket),
        game_commands(game_queue),
        running(false),
        is_dead(false) {}

void ClientHandler::start() {
    running = true;
    receiver_thread = Thread([this]() { this->receiver_loop(); });
    sender_thread = Thread([this]() { this->sender_loop(); });
}

void ClientHandler::stop() {
    running = false;
    send_queue.close();
}

void ClientHandler::join() {
    receiver_thread.join();
    sender_thread.join();
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
        // Cliente desconectado o error de red
        running = false;
        is_dead.store(true);  // Escritura atómica
        send_queue.close();   // Despertamos al sender
    }
}

//////////////////////// THREAD SENDER ////////////////////////

void ClientHandler::sender_loop() {
    try {
        while (running) {
            std::optional<NitroEvent> event = send_queue.pop();

            if (!event.has_value()) {
                break;  // Queue cerrada
            }

            protocol.send_nitro_event(event.value());
        }
    } catch (const std::exception& e) {
        // Error al enviar, cliente probablemente desconectado
        running = false;
        is_dead.store(true);  // Escritura atómica
    }
}

//////////////////////// ENVIAR EVENTO ////////////////////////

void ClientHandler::send_event(const NitroEvent& event) {
    // Lectura atómica de is_dead
    if (!is_dead.load(std::memory_order_acquire)) {
        // BlockingQueue::push maneja queues cerradas, pero agregamos
        // try-catch por seguridad ante posible race condition
        try {
            send_queue.push(NitroEvent(event));
        } catch (const std::exception& e) {
            // Si la queue se cerró justo ahora, ignoramos silenciosamente
            // Esto es esperado cuando un cliente se desconecta
        }
    }
}
