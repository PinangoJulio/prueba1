#ifndef GAMELOOP_H
#define GAMELOOP_H

#include <atomic>

#include "../common_src/common_thread.h"

#include "clients_monitor.h"
#include "game_logic.h"

// 4 loops por segundo = 250ms por loop
// se puede MODIFICAR a conveniencia
#define GAMELOOP_SLEEP_MS 250

class GameLoop {
private:
    GameLogic game_logic;
    ClientsMonitor& clients_monitor;
    std::atomic<bool> running;
    Thread loop_thread;

    //////////////////////// LOOP PRINCIPAL ////////////////////////

    void loop();
    void handle_game_event(const GameEvent& game_event);

public:
    GameLoop(ClientsMonitor& monitor, NonBlockingQueue<GameCommand>& commands);

    void start();

    void stop();

    GameLoop(const GameLoop&) = delete;
    GameLoop& operator=(const GameLoop&) = delete;
};

#endif  // GAMELOOP_H
