#ifndef CAR_H
#define CAR_H

#include <cstdint>


// 3 segundos = 12 iteraciones (4 loops/segundo)
// se puede MODIFICAR a conveniencia
#define NITRO_DURATION_ITERATIONS 12


class Car {
private:
    bool nitro_active;
    int iterations_remaining;

public:
    Car(): nitro_active(false), iterations_remaining(0) {}

    bool activate_nitro() {
        if (nitro_active) {
            return false;  // Se ignora, el nitro ya está activo, no deberia poder usarlo
        }
        nitro_active = true;
        iterations_remaining = NITRO_DURATION_ITERATIONS;
        return true;
    }

    bool update() {
        if (!nitro_active) {
            return false;
        }

        iterations_remaining--;

        if (iterations_remaining <= 0) {
            nitro_active = false;
            iterations_remaining = 0;
            return true;  // El nitro ya expiró, puede volver a usarlo
        }

        return false;  // El nitro sigue activo, no puede usarlo
    }

    bool will_expire() const {
        if (!nitro_active) {
            return false;
        }
        return iterations_remaining == 1;
    }

    bool has_nitro() const { return nitro_active; }

    void reset() {
        nitro_active = false;
        iterations_remaining = 0;
    }
};

#endif  // CAR_H
