#include "acceptor.h"

#include <exception>
#include <utility>

//////////////////////// CONSTRUCTOR ////////////////////////

Acceptor::Acceptor(const char* port): acceptor_socket(port), running(false) {}

void Acceptor::start(std::function<void(Socket)> callback) {
    running.store(true, std::memory_order_release);
    on_new_client = std::move(callback);
    acceptor_thread = Thread([this]() { this->acceptor_loop(); });
}

void Acceptor::stop() {
    running.store(false, std::memory_order_release);

    try {
        acceptor_socket.shutdown(2);
    } catch (const std::exception& e) {
        try {
            acceptor_socket.close();
        } catch (...) {
            // se van aignorar los errores de cierre
        }
    }

    try {
        acceptor_socket.close();
    } catch (const std::exception& e) {
        // se van aignorar los errores de cierre
    }

    acceptor_thread.join();
}

//////////////////////// LOOP DEL ACCEPTOR ////////////////////////

void Acceptor::acceptor_loop() {
    while (running.load(std::memory_order_acquire)) {
        try {
            Socket client_socket = acceptor_socket.accept();

            if (!running.load(std::memory_order_acquire)) {
                break;
            }

            on_new_client(std::move(client_socket));
        } catch (const std::exception& e) {
            if (!running.load(std::memory_order_acquire)) {
                break;
            }
        }
    }
}
