# TP Threads - Need for Speed Nitro PoC

## Descripción
Este proyecto implementa una prueba de concepto (PoC) del sistema de nitro para el juego Need for Speed. El servidor gestiona una partida multijugador donde los clientes pueden activar el nitro de sus autos, aumentando su velocidad temporalmente. El sistema utiliza múltiples threads para manejar la comunicación, el game loop y la lógica del juego de forma concurrente.

## Estructura del Proyecto
``` 
├── client_src/
│   ├── client.h
│   ├── client.cpp
│   └── client_main.cpp
├── server_src/
│   ├── server.h
│   ├── server.cpp
│   ├── server_main.cpp
│   ├── acceptor.h
│   ├── acceptor.cpp
│   ├── client_handler.h
│   ├── client_handler.cpp
│   ├── clients_monitor.h
│   ├── gameloop.h
│   ├── gameloop.cpp
│   ├── game_logic.h
│   ├── game_logic.cpp
│   └── car.h
├── common_src/
│   ├── common_protocol_threads.h
│   ├── common_queue.h
│   ├── common_thread.h
│   ├── common_socket.*
│   ├── resolver.*
│   ├── liberror.*
│   └── resolvererror.*
├── MakefileThreads
└── README.md
```

## Arquitectura

### Cliente
El cliente opera con un único thread que lee comandos de stdin, envía mensajes al servidor y recibe eventos.

### Servidor
El servidor utiliza múltiples threads:
- **Main Thread**: Lee stdin esperando el comando `q` para shutdown
- **Acceptor Thread**: Acepta nuevas conexiones de clientes
- **GameLoop Thread**: Ejecuta 4 iteraciones por segundo procesando comandos, simulando el mundo y haciendo broadcast de eventos
- **Receiver/Sender Threads**: 2 threads por cada cliente conectado para comunicación asíncrona

### Sincronización
- **NonBlockingQueue**: Para comandos del game loop
- **BoundedBlockingQueue**: Para mensajes a enviar por cliente (bounded, bloqueante en lectura, no bloqueante en escritura con `try_push`)
- **ClientsMonitor**: Monitor thread-safe que protege la colección de clientes

## Lógica del Juego

### Sistema de Nitro
- Duración: 3 segundos (12 iteraciones del game loop a 4 loops/segundo)
- Si un auto intenta activar el nitro mientras ya está activo, se ignora el comando
- Al activarse o expirar el nitro, se envía evento a todos los clientes

### Game Loop
1. Procesar comandos pendientes de los clientes
2. Simular mundo (actualizar estado de nitros)
3. Limpiar threads muertos (RIP)
4. Sleep de 250ms

## Decisiones de Diseño

### Separación de Responsabilidades
- **GameLogic**: Contiene la lógica del juego (activar nitro, simular mundo)
- **GameLoop**: Orquesta el loop principal sin bloquearse
- **ClientsMonitor**: Maneja la colección de clientes con critical sections explícitas
- **Acceptor**: Acepta conexiones (ownership del socket aceptador)

### Justificación de Queues
- **NonBlockingQueue (comandos)**: Unbounded y no bloqueante porque el gameloop no debería poder bloquearse esperando comandos. Se usa `try_pop()` para procesar todos los comandos disponibles sin esperar.
- **BoundedBlockingQueue (eventos)**: Bounded para limitar memoria. Se usa `try_push()` en broadcast para no bloquear el gameloop si la queue de un cliente está llena. El sender usa `pop()` bloqueante porque debería esperar eventos para enviar.


## Compilación

# Compilación normal
make -f MakefileThreads

# Con simulación de problemas de red
make -f MakefileThreads wrapsocks=1

## Ejecucion de pruebas

# Ejecutar casos de prueba
./run_tests.sh . casos/ multi-client no-valgrind 60 10 yes

# Comparar salidas
./compare_outputs.sh casos/ salidas/

##  Implementación Realizada
- **Socket y comunicación de red**: Implementación provista por la cátedra
  - Fuente: https://github.com/eldipa/sockets-en-cpp
  - Licencia: Ver repositorio original
- **Ideas para el thread y la queue**: Implementacion provista por la cátedra
  - Fuente: https://github.com/eldipa/hands-on-threads
  - Licencia: Ver repositorio original

- **Protocolo, Cliente y Servidor**
- **Arquitectura multi-threading:** Diseño e implementación propia de acuerdo al enunciado del TP
- **Protocolo de comunicación:** Implementación según especificación del enunciado del TP
- **Queues thread-safe:** Implementación con semántica y arctitectura blocking/non-blocking
- **Monitor de clientes:** Patrón monitor con critical sections
- **Game loop y lógica del juego:** Separación de responsabilidades
