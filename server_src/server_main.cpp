#include <iostream>
#include <exception>
#include <string>
#include "server.h"

#define EXPECTED_ARGC 2
#define SUCCESS_CODE 0
#define ERROR_CODE 1

int main(int argc, const char* argv[]) {
    if (argc != EXPECTED_ARGC) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return ERROR_CODE;
    }

    std::string port = argv[1];

    try {
        Server server(port);
        server.start();
        
        // Esperar a que se ingrese 'q' para detener
        std::string input;
        while (std::getline(std::cin, input)) {
            if (input == "q") {
                break;
            }
        }
        
        server.stop();
        server.wait_for_finish();
        
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return ERROR_CODE;
    }

    return SUCCESS_CODE;
}