#ifndef CAR_H
#define CAR_H

#include <cstdint>

// Constante: 3 segundos = 12 iteraciones (4 loops/segundo)
#define NITRO_DURATION_ITERATIONS 12

class Car {
private:
    bool nitro_active;
    int iterations_remaining;  // Iteraciones restantes de nitro

public:
    Car(): nitro_active(false), iterations_remaining(0) {}

    // Activar nitro
    bool activate_nitro() {
        if (nitro_active) {
            return false;  // Ya tiene nitro activo, se ignora
        }
        nitro_active = true;
        iterations_remaining = NITRO_DURATION_ITERATIONS;
        return true;  // Nitro activado exitosamente
    }

    // Actualizar estado (llamado en cada iteración del gameloop)
    // Retorna true si el nitro expiró en esta iteración
    bool update() {
        if (!nitro_active) {
            return false;
        }

        iterations_remaining--;
        
        if (iterations_remaining <= 0) {
            nitro_active = false;
            iterations_remaining = 0;
            return true;  // Nitro expiró
        }
        
        return false;
    }

    bool has_nitro() const {
        return nitro_active;
    }

    void reset() {
        nitro_active = false;
        iterations_remaining = 0;
    }
};

#endif  // CAR_H
