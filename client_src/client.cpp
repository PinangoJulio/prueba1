#include "client.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

Client::Client(const std::string& hostname, const std::string& port):
        socket(hostname.c_str(), port.c_str()), protocol(socket) {}

void Client::run() {
    process_commands();
}

//////////////////////// PROCESAMIENTO DE COMANDOS ////////////////////////

void Client::process_commands() {
    std::string line;
    
    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            continue;
        }

        std::istringstream iss(line);
        std::string command;
        iss >> command;

        std::string parameter;
        iss >> parameter;

        execute_command(command, parameter);

        // Si es exit, salir del loop
        if (command == "exit") {
            break;
        }
    }
}

void Client::execute_command(const std::string& command, const std::string& parameter) {
    if (command == "nitro") {
        handle_nitro();
    } else if (command == "read") {
        int count = std::stoi(parameter);
        handle_read(count);
    } else if (command == "exit") {
        // No hace nada, solo sale del loop
    } else {
        std::cerr << "Unknown command: " << command << std::endl;
    }
}

void Client::handle_nitro() {
    try {
        protocol.send_activate_nitro();
    } catch (const std::exception& e) {
        std::cerr << "Error sending nitro: " << e.what() << std::endl;
    }
}

void Client::handle_read(int count) {
    try {
        for (int i = 0; i < count; i++) {
            uint8_t cmd = protocol.receive_command();
            
            if (cmd == CMD_NITRO_EVENT) {
                NitroEvent event = protocol.receive_nitro_event();
                print_nitro_event(event);
            } else {
                std::cerr << "Unknown command received: 0x" << std::hex 
                          << (int)cmd << std::dec << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error reading events: " << e.what() << std::endl;
    }
}

void Client::print_nitro_event(const NitroEvent& event) {
    if (event.event_type == EVENT_NITRO_ACTIVATED) {
        std::cout << "A car hit the nitro!" << std::endl;
    } else if (event.event_type == EVENT_NITRO_EXPIRED) {
        std::cout << "A car is out of juice." << std::endl;
    }
}