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

    try {
        acceptor_socket.shutdown(2);
        acceptor_socket.close();
    } catch (const std::exception& e) {
        // Ignoramos errores al cerrar el socket
    }

    acceptor_thread.join();
}

void Acceptor::acceptor_loop() {
    while (running) {
        try {
            Socket client_socket = acceptor_socket.accept();
            on_new_client(std::move(client_socket));
        } catch (const std::exception& e) {
            // Si running es false, es un cierre esperado
            if (!running) {
                break;
            }
            // Otro error, continuamos intentando aceptar
        }
    }
}
