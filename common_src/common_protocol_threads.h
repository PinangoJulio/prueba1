#ifndef COMMON_PROTOCOL_THREADS_H
#define COMMON_PROTOCOL_THREADS_H

#include <cstdint>
#include <stdexcept>

#include <arpa/inet.h>

#include "common_socket.h"

//////////////////////// PROTOCOLO ////////////////////////

constexpr uint8_t CMD_ACTIVATE_NITRO = 0x04;
constexpr uint8_t CMD_NITRO_EVENT = 0x10;

constexpr uint8_t EVENT_NITRO_ACTIVATED = 0x07;
constexpr uint8_t EVENT_NITRO_EXPIRED = 0x08;

//////////////////////// DTOs ////////////////////////

struct NitroEvent {
    uint16_t cars_with_nitro;
    uint8_t event_type;

    NitroEvent(): cars_with_nitro(0), event_type(0) {}
    NitroEvent(uint16_t cars, uint8_t type): cars_with_nitro(cars), event_type(type) {}
};

//////////////////////// PROTOCOLO ////////////////////////

class ProtocolThreads {
private:
    Socket& socket;

public:
    explicit ProtocolThreads(Socket& skt): socket(skt) {}

    //////////////////////// ENVÍO ////////////////////////

    void send_activate_nitro() {
        uint8_t cmd = CMD_ACTIVATE_NITRO;
        socket.sendall(&cmd, sizeof(cmd));
    }

    void send_nitro_event(const NitroEvent& event) {
        uint8_t cmd = CMD_NITRO_EVENT;
        socket.sendall(&cmd, sizeof(cmd));

        uint16_t cars_network = htons(event.cars_with_nitro);
        socket.sendall(&cars_network, sizeof(cars_network));

        socket.sendall(&event.event_type, sizeof(event.event_type));
    }

    //////////////////////// RECEPCIÓN ////////////////////////

    uint8_t receive_command() {
        uint8_t cmd;
        int ret = socket.recvall(&cmd, sizeof(cmd));
        if (ret == 0) {
            throw std::runtime_error("Connection closed");
        }
        return cmd;
    }

    NitroEvent receive_nitro_event() {
        uint16_t cars_network;
        socket.recvall(&cars_network, sizeof(cars_network));
        uint16_t cars = ntohs(cars_network);

        uint8_t event_type;
        socket.recvall(&event_type, sizeof(event_type));

        return NitroEvent(cars, event_type);
    }

    ProtocolThreads(const ProtocolThreads&) = delete;
    ProtocolThreads& operator=(const ProtocolThreads&) = delete;
};

#endif  // COMMON_PROTOCOL_THREADS_H
