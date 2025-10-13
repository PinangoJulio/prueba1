#ifndef CAR_H
#define CAR_H

#include <cstdint>

//////////////////////// CONFIGURACIÓN ////////////////////////

// 3 segundos = 12 iteraciones (4 loops/segundo)
// Puedes modificar este valor según necesites
#define NITRO_DURATION_ITERATIONS 12

//////////////////////// CAR ////////////////////////

class Car {
private:
    bool nitro_active;
    int iterations_remaining;

public:
    //////////////////////// CONSTRUCTOR ////////////////////////

    Car(): nitro_active(false), iterations_remaining(0) {}

    //////////////////////// ACTIVACIÓN DE NITRO ////////////////////////

    // Intenta activar el nitro
    // Retorna true si se activó, false si ya estaba activo
    bool activate_nitro() {
        if (nitro_active) {
            return false;  // Se ignora porque el nitro ya está activo
        }
        nitro_active = true;
        iterations_remaining = NITRO_DURATION_ITERATIONS;
        return true;
    }

    //////////////////////// ACTUALIZACIÓN ////////////////////////

    // Actualiza el estado del nitro (decrementa el contador)
    // Retorna true si el nitro expiró en esta iteración
    bool update() {
        if (!nitro_active) {
            return false;
        }

        iterations_remaining--;

        if (iterations_remaining <= 0) {
            nitro_active = false;
            iterations_remaining = 0;
            return true;  // El nitro expiró
        }

        return false;  // El nitro sigue activo
    }

    //////////////////////// CONSULTAS ////////////////////////

    // Verifica si el nitro va a expirar en el próximo update (sin actualizar)
    bool will_expire() const {
        if (!nitro_active) {
            return false;
        }
        return iterations_remaining == 1;
    }

    // Verifica si el nitro está activo
    bool has_nitro() const { return nitro_active; }

    //////////////////////// RESET ////////////////////////

    // Resetea el estado del auto
    void reset() {
        nitro_active = false;
        iterations_remaining = 0;
    }
};

#endif  // CAR_H
