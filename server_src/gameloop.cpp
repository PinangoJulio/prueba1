#include "gameloop.h"

#include <chrono>
#include <iostream>
#include <thread>

//////////////////////// CONSTRUCTOR ////////////////////////

GameLoop::GameLoop(ClientsMonitor& monitor, NonBlockingQueue<GameCommand>& commands):
        game_logic(monitor, commands), clients_monitor(monitor), running(false) {

    // Configuramos el callback para manejar eventos del juego
    game_logic.set_event_callback(
            [this](const GameEvent& event) { this->handle_game_event(event); });
}

//////////////////////// CONTROL DE CICLO DE VIDA ////////////////////////

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
        // 1. Procesar comandos de los clientes
        game_logic.process_commands();

        // 2. Simular el mundo (actualizar nitros)
        game_logic.simulate_world();

        // 3. Limpiar clientes muertos (RIP)
        clients_monitor.remove_dead_clients();

        // 4. Sleep (única llamada a sleep en todo el gameloop)
        std::this_thread::sleep_for(std::chrono::milliseconds(GAMELOOP_SLEEP_MS));
    }
}

void GameLoop::handle_game_event(const GameEvent& game_event) {
    // Imprimimos el mensaje si corresponde
    if (game_event.should_print) {
        std::cout << game_event.message << std::endl;
    }

    // Hacemos broadcast del evento a todos los clientes vivos
    clients_monitor.apply_to_all(
            [&](ClientHandler& client) { client.send_event(game_event.event); });
}
