#include "acceptor.h"

#include <exception>
#include <utility>

//////////////////////// CONSTRUCTOR ////////////////////////

Acceptor::Acceptor(const char* port): acceptor_socket(port), running(false) {}

//////////////////////// CONTROL DE CICLO DE VIDA ////////////////////////

void Acceptor::start(std::function<void(Socket)> callback) {
    running.store(true, std::memory_order_release);
    on_new_client = std::move(callback);
    acceptor_thread = Thread([this]() { this->acceptor_loop(); });
}

void Acceptor::stop() {
    running.store(false, std::memory_order_release);

    // Orden correcto: shutdown -> close -> join
    // El shutdown despierta al accept() bloqueado
    try {
        acceptor_socket.shutdown(2);
    } catch (const std::exception& e) {
        // Si falla shutdown, intentamos cerrar directamente
        try {
            acceptor_socket.close();
        } catch (...) {
            // Ignoramos errores de cierre
        }
    }

    try {
        acceptor_socket.close();
    } catch (const std::exception& e) {
        // Ignoramos errores al cerrar
    }

    // Ahora esperamos a que el thread termine
    acceptor_thread.join();
}

//////////////////////// LOOP DEL ACCEPTOR ////////////////////////

void Acceptor::acceptor_loop() {
    while (running.load(std::memory_order_acquire)) {
        try {
            Socket client_socket = acceptor_socket.accept();

            // Verificamos running después de accept por si nos despertaron con el cierre
            if (!running.load(std::memory_order_acquire)) {
                break;
            }

            on_new_client(std::move(client_socket));
        } catch (const std::exception& e) {
            // Si running es false, es un cierre esperado
            if (!running.load(std::memory_order_acquire)) {
                break;
            }
            // Otro error, podría ser temporal, continuamos
        }
    }
}
