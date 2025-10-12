#include "acceptor.h"

#include <exception>
#include <utility>

Acceptor::Acceptor(const char* port): acceptor_socket(port), running(false) {}

void Acceptor::start(std::function<void(Socket)> callback) {
    running = true;
    on_new_client = std::move(callback);
    acceptor_thread = Thread([this]() { this->acceptor_loop(); });
}

void Acceptor::stop() {
    running = false;

    // Primero cerramos el socket para despertar al accept()
    // El orden correcto es: shutdown -> close -> join
    try {
        acceptor_socket.shutdown(2);
    } catch (const std::exception& e) {
        // Si falla el shutdown, intentamos cerrar directamente
    }

    try {
        acceptor_socket.close();
    } catch (const std::exception& e) {
        // Ignoramos errores al cerrar
    }

    // Ahora sí esperamos a que el thread termine
    acceptor_thread.join();
}

void Acceptor::acceptor_loop() {
    while (running) {
        try {
            Socket client_socket = acceptor_socket.accept();

            // Verificamos running después de accept por si nos despertaron con el cierre
            if (!running) {
                break;
            }

            on_new_client(std::move(client_socket));
        } catch (const std::exception& e) {
            // Si running es false, es un cierre esperado
            if (!running) {
                break;
            }
            // Otro error, podría ser temporal, continuamos
        }
    }
}
