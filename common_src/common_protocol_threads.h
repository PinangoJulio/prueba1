#ifndef COMMON_PROTOCOL_THREADS_H
#define COMMON_PROTOCOL_THREADS_H

#include <cstdint>
#include <arpa/inet.h>
#include "common_socket.h"
#include <stdexcept>

// Constantes del protocolo de threads
#define CMD_ACTIVATE_NITRO 0x04
#define CMD_NITRO_EVENT 0x10
#define EVENT_NITRO_ACTIVATED 0x07
#define EVENT_NITRO_EXPIRED 0x08

// Mensaje de evento de nitro
struct NitroEvent {
    uint16_t cars_with_nitro;  // Cantidad de autos con nitro activo
    uint8_t event_type;         // 0x07 = activado, 0x08 = expirado
    
    NitroEvent(): cars_with_nitro(0), event_type(0) {}
    NitroEvent(uint16_t cars, uint8_t type): 
        cars_with_nitro(cars), event_type(type) {}
};

class ProtocolThreads {
private:
    Socket& socket;

public:
    explicit ProtocolThreads(Socket& skt): socket(skt) {}

    //////////////////////// ENVÍO ////////////////////////
    
    // Cliente envía comando de activar nitro
    void send_activate_nitro() {
        uint8_t cmd = CMD_ACTIVATE_NITRO;
        socket.sendall(&cmd, sizeof(cmd));
    }

    // Servidor envía evento de nitro
    void send_nitro_event(const NitroEvent& event) {
        uint8_t cmd = CMD_NITRO_EVENT;
        socket.sendall(&cmd, sizeof(cmd));
        
        uint16_t cars_network = htons(event.cars_with_nitro);
        socket.sendall(&cars_network, sizeof(cars_network));
        
        socket.sendall(&event.event_type, sizeof(event.event_type));
    }

    //////////////////////// RECEPCIÓN ////////////////////////
    
    // Recibir comando (devuelve código de comando)
    uint8_t receive_command() {
        uint8_t cmd;
        int ret = socket.recvall(&cmd, sizeof(cmd));
        if (ret == 0) {
            throw std::runtime_error("Connection closed");
        }
        return cmd;
    }

    // Recibir evento de nitro
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