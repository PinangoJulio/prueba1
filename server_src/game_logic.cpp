#include "game_logic.h"

#include <algorithm>
#include <iostream>


GameLogic::GameLogic(ClientsMonitor& monitor, NonBlockingQueue<GameCommand>& commands):
        clients_monitor(monitor), game_commands(commands) {}

void GameLogic::set_event_callback(std::function<void(const GameEvent&)> callback) {
    on_event = callback;
}

//////////////////////// LÓGICA DEL JUEGO ////////////////////////

void GameLogic::process_commands() {
    while (true) {
        std::optional<GameCommand> cmd = game_commands.try_pop();

        if (!cmd.has_value()) {
            break;
        }

        bool activated = false;
        clients_monitor.apply_to_client(cmd->client_id, [&](ClientHandler& client) {
            activated = client.get_car().activate_nitro();
        });

        if (activated) {
            uint16_t cars_with_nitro = count_cars_with_nitro();
            NitroEvent event(cars_with_nitro, EVENT_NITRO_ACTIVATED);

            if (on_event) {
                GameEvent game_event(event, true, "A car hit the nitro!");
                on_event(game_event);
            }
        }
    }
}

// Identificamos qué autos van a expirar ANTES de actualizar y actualizamos los autos que NO van a
// terminar y justo para cada auto de eso actualizamos y le mandamos el evento

void GameLogic::simulate_world() {
    std::vector<int> about_to_expire;
    std::vector<int> all_clients;

    clients_monitor.apply_to_all([&](ClientHandler& client) {
        all_clients.push_back(client.get_id());
        if (client.get_car().will_expire()) {
            about_to_expire.push_back(client.get_id());
        }
    });

    for (int client_id: all_clients) {
        bool will_expire = std::any_of(about_to_expire.begin(), about_to_expire.end(),
                                       [client_id](int id) { return id == client_id; });

        if (!will_expire) {
            clients_monitor.apply_to_client(
                    client_id, [&](ClientHandler& client) { client.get_car().update(); });
        }
    }

    for (int client_id: about_to_expire) {
        clients_monitor.apply_to_client(client_id,
                                        [&](ClientHandler& client) { client.get_car().update(); });

        uint16_t cars_with_nitro = count_cars_with_nitro();
        NitroEvent event(cars_with_nitro, EVENT_NITRO_EXPIRED);

        if (on_event) {
            GameEvent game_event(event, true, "A car is out of juice.");
            on_event(game_event);
        }
    }
}

uint16_t GameLogic::count_cars_with_nitro() {
    uint16_t count = 0;
    clients_monitor.apply_to_all([&](ClientHandler& client) {
        if (client.get_car().has_nitro()) {
            count++;
        }
    });
    return count;
}
