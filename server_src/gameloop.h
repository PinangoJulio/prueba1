#ifndef GAMELOOP_H
#define GAMELOOP_H

#include <atomic>

#include "../common_src/common_thread.h"

#include "clients_monitor.h"
#include "game_logic.h"

//////////////////////// CONFIGURACIÓN ////////////////////////

// 4 loops por segundo = 250ms por loop
#define GAMELOOP_SLEEP_MS 250

//////////////////////// GAMELOOP ////////////////////////

class GameLoop {
private:
    GameLogic game_logic;
    ClientsMonitor& clients_monitor;
    std::atomic<bool> running;
    Thread loop_thread;

    //////////////////////// LOOP PRINCIPAL ////////////////////////

    // Loop principal del juego
    void loop();

    // Maneja un evento del juego (broadcast y print)
    void handle_game_event(const GameEvent& game_event);

public:
    //////////////////////// CONSTRUCTOR ////////////////////////

    GameLoop(ClientsMonitor& monitor, NonBlockingQueue<GameCommand>& commands);

    //////////////////////// CONTROL DE CICLO DE VIDA ////////////////////////

    // Inicia el gameloop
    void start();

    // Detiene el gameloop
    void stop();

    //////////////////////// MOVIMIENTO Y COPIA ////////////////////////

    GameLoop(const GameLoop&) = delete;
    GameLoop& operator=(const GameLoop&) = delete;
};

#endif  // GAMELOOP_H
