#ifndef CLIENT_H
#define CLIENT_H

#include <string>

#include "../common_src/common_protocol_threads.h"
#include "../common_src/common_socket.h"

class Client {
private:
    Socket socket;
    ProtocolThreads protocol;

    //////////////////////// PROCESAMIENTO DE COMANDOS ////////////////////////

    void process_commands();

    void execute_command(const std::string& command, const std::string& parameter);

    void handle_nitro();

    void handle_read(int count);

    void print_nitro_event(const NitroEvent& event);

public:
    Client(const std::string& hostname, const std::string& port);

    void run();

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
};

#endif  // CLIENT_H
