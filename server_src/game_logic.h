#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <cstdint>
#include <functional>
#include <vector>

#include "../common_src/common_protocol_threads.h"
#include "../common_src/common_queue.h"

#include "client_handler.h"
#include "clients_monitor.h"

// Estructura para eventos que deben ser broadcasted
struct GameEvent {
    NitroEvent event;
    bool should_print;
    const char* message;

    GameEvent(const NitroEvent& e, bool print, const char* msg):
            event(e), should_print(print), message(msg) {}
};

class GameLogic {
private:
    ClientsMonitor& clients_monitor;
    NonBlockingQueue<GameCommand>& game_commands;

    // Callback para enviar eventos (desacopla de cómo se envían)
    std::function<void(const GameEvent&)> on_event;

    uint16_t count_cars_with_nitro();

public:
    GameLogic(ClientsMonitor& monitor, NonBlockingQueue<GameCommand>& commands);

    // Configura el callback para eventos
    void set_event_callback(std::function<void(const GameEvent&)> callback);

    // Procesa todos los comandos pendientes
    void process_commands();

    // Simula una iteración del mundo
    void simulate_world();

    GameLogic(const GameLogic&) = delete;
    GameLogic& operator=(const GameLogic&) = delete;
};

#endif  // GAME_LOGIC_H
