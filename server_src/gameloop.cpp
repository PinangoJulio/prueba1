#include "gameloop.h"

#include <chrono>
#include <iostream>
#include <thread>

GameLoop::GameLoop(ClientsMonitor& monitor, NonBlockingQueue<GameCommand>& commands):
        game_logic(monitor, commands), clients_monitor(monitor), running(false) {

    // Configuramos el callback para manejar eventos del juego
    game_logic.set_event_callback(
            [this](const GameEvent& event) { this->handle_game_event(event); });
}

void GameLoop::start() {
    running = true;
    loop_thread = Thread([this]() { this->loop(); });
}

void GameLoop::stop() {
    running = false;
    loop_thread.join();
}

void GameLoop::loop() {
    while (running) {
        game_logic.process_commands();
        game_logic.simulate_world();
        std::this_thread::sleep_for(std::chrono::milliseconds(GAMELOOP_SLEEP_MS));
    }
}

void GameLoop::handle_game_event(const GameEvent& game_event) {
    // Imprimimos el mensaje si corresponde
    if (game_event.should_print) {
        std::cout << game_event.message << std::endl;
    }

    // Hacemos broadcast del evento
    clients_monitor.apply_to_all(
            [&](ClientHandler& client) { client.send_event(game_event.event); });
}
