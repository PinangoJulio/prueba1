#include "gameloop.h"

#include <chrono>
#include <iostream>
#include <thread>

GameLoop::GameLoop(ClientsMonitor& monitor, NonBlockingQueue<GameCommand>& commands):
        game_logic(monitor, commands), clients_monitor(monitor), running(false) {

    game_logic.set_event_callback(
            [this](const GameEvent& event) { this->handle_game_event(event); });
}

void GameLoop::start() {
    running.store(true, std::memory_order_release);
    loop_thread = Thread([this]() { this->loop(); });
}

void GameLoop::stop() {
    running.store(false, std::memory_order_release);
    loop_thread.join();
}

//////////////////////// LOOP PRINCIPAL ////////////////////////

void GameLoop::loop() {
    while (running.load(std::memory_order_acquire)) {
        // Procesar los comandos de los clientes
        game_logic.process_commands();

        // Simular el mundo (actualizar los nitros)
        game_logic.simulate_world();

        // Limpiar clientes muertos (hace el RIP)
        clients_monitor.remove_dead_clients();

        // llamada al Sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(GAMELOOP_SLEEP_MS));
    }
}

void GameLoop::handle_game_event(const GameEvent& game_event) {
    if (game_event.should_print) {
        std::cout << game_event.message << std::endl;
    }

    clients_monitor.apply_to_all(
            [&](ClientHandler& client) { client.send_event(game_event.event); });
}
