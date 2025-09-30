#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include "../common_src/common_socket.h"
#include "../common_src/common_protocol_threads.h"

class Client {
private:
    Socket socket;
    ProtocolThreads protocol;

    //////////////////////// PROCESAMIENTO DE COMANDOS ////////////////////////
    
    // Lee y procesa comandos desde stdin
    void process_commands();
    
    // Ejecuta comando individual
    void execute_command(const std::string& command, const std::string& parameter);
    
    // Comando: nitro
    void handle_nitro();
    
    // Comando: read N
    void handle_read(int count);
    
    // Imprime evento de nitro recibido
    void print_nitro_event(const NitroEvent& event);

public:
    // Constructor: conecta al servidor
    Client(const std::string& hostname, const std::string& port);

    // Main loop del cliente
    void run();

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
};

#endif  // CLIENT_H