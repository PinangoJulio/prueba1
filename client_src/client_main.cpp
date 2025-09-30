#include <iostream>
#include <exception>
#include "client.h"

#define EXPECTED_ARGC 3
#define SUCCESS_CODE 0
#define ERROR_CODE 1

int main(int argc, const char* argv[]) {
    if (argc != EXPECTED_ARGC) {
        std::cerr << "Usage: " << argv[0] << " <hostname> <port>" << std::endl;
        return ERROR_CODE;
    }

    std::string hostname = argv[1];
    std::string port = argv[2];

    try {
        Client client(hostname, port);
        client.run();
    } catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
        return ERROR_CODE;
    }

    return SUCCESS_CODE;
}