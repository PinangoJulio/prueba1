#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <atomic>
#include <functional>

#include "../common_src/common_socket.h"
#include "../common_src/common_thread.h"

//////////////////////// ACCEPTOR ////////////////////////

class Acceptor {
private:
    Socket acceptor_socket;
    Thread acceptor_thread;
    std::atomic<bool> running;
    std::function<void(Socket)> on_new_client;

    //////////////////////// LOOP DEL ACCEPTOR ////////////////////////

    // Loop que acepta clientes entrantes
    void acceptor_loop();

public:
    //////////////////////// CONSTRUCTOR ////////////////////////

    explicit Acceptor(const char* port);

    //////////////////////// CONTROL DE CICLO DE VIDA ////////////////////////

    // Inicia el thread aceptador con el callback para nuevos clientes
    void start(std::function<void(Socket)> callback);

    // Detiene el thread aceptador
    void stop();

    //////////////////////// MOVIMIENTO Y COPIA ////////////////////////

    Acceptor(const Acceptor&) = delete;
    Acceptor& operator=(const Acceptor&) = delete;
};

#endif  // ACCEPTOR_H
