#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <cstdint>
#include <functional>
#include <vector>

#include "../common_src/common_protocol_threads.h"
#include "../common_src/common_queue.h"

#include "client_handler.h"
#include "clients_monitor.h"

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

    std::function<void(const GameEvent&)> on_event;

    uint16_t count_cars_with_nitro();

public:
    GameLogic(ClientsMonitor& monitor, NonBlockingQueue<GameCommand>& commands);

    void set_event_callback(std::function<void(const GameEvent&)> callback);

    void process_commands();

    void simulate_world();

    GameLogic(const GameLogic&) = delete;
    GameLogic& operator=(const GameLogic&) = delete;
};

#endif  // GAME_LOGIC_H
