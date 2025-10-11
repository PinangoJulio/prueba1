#ifndef GAMELOOP_THREAD_H
#define GAMELOOP_THREAD_H

#include <chrono>
#include <iostream>
#include <thread>

#include "../common_src/queue.h"
#include "../common_src/thread.h"

#include "client_handler.h"
#include "monitor_clients.h"

#define GAMELOOP_SLEEP_MS 250  // 4 loops/segundo

/*
 * Thread GameLoop
 *
 * RESPONSABILIDAD:
 * - Ejecutar la lógica del juego
 * - Procesar comandos de los clientes
 * - Simular el mundo (expirar nitros)
 * - Hacer broadcast de eventos
 */
class GameLoopThread: public Thread {
private:
    MonitorClients& monitor;
    Queue<GameCommand>& game_commands;

    //////////////////////// LÓGICA DEL JUEGO ////////////////////////

    void process_commands() {
        // Leer TODOS los comandos pendientes (non-blocking)
        GameCommand cmd;
        while (game_commands.try_pop(cmd)) {
            activate_nitro_for_client(cmd.client_id);
        }
    }

    void activate_nitro_for_client(int client_id) {
        monitor.for_each_client([&](ClientHandler* client) {
            if (client->get_id() == client_id) {
                bool activated = client->get_car().activate_nitro();

                if (activated) {
                    uint16_t cars_with_nitro = count_cars_with_nitro();
                    NitroEvent event(cars_with_nitro, EVENT_NITRO_ACTIVATED);

                    std::cout << "A car hit the nitro!" << std::endl;
                    monitor.broadcast(event);
                }
            }
        });
    }

    void simulate_world() {
        monitor.for_each_client([&](ClientHandler* client) {
            bool expired = client->get_car().update();

            if (expired) {
                uint16_t cars_with_nitro = count_cars_with_nitro();
                NitroEvent event(cars_with_nitro, EVENT_NITRO_EXPIRED);

                std::cout << "A car is out of juice." << std::endl;
                monitor.broadcast(event);
            }
        });
    }

    uint16_t count_cars_with_nitro() {
        uint16_t count = 0;
        monitor.for_each_client([&](ClientHandler* client) {
            if (client->get_car().has_nitro()) {
                count++;
            }
        });
        return count;
    }

public:
    GameLoopThread(MonitorClients& mon, Queue<GameCommand>& queue):
            monitor(mon), game_commands(queue) {}

    void run() override {
        while (should_keep_running()) {
            // Paso 1: Procesar comandos
            process_commands();

            // Paso 2: Simular mundo
            simulate_world();

            // Paso 3: Reaper - limpiar clientes muertos
            monitor.remove_dead_clients();

            // Paso 4: Sleep (fragmentado para salir rápido)
            for (int i = 0; i < 25 && should_keep_running(); i++) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }
};

#endif  // GAMELOOP_THREAD_H
