[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/XglgMq0o)
# Documentación del Proyecto
---
Estudiante:  
Id:  
---
### 1. ¿Cuál es la diferencia entre un programa y un proceso?
Un programa es solo un archivo con instrucciones (como un archivo .exe).
Un proceso es cuando el programa está corriendo en la computadora, usando memoria y CPU.
### 2. ¿Puedo tener múltiples procesos corriendo el mismo programa?
Sí, puedes tener muchos procesos ejecutando el mismo programa al mismo tiempo. Ejemplo: abrir varias ventanas del navegador.

### 3. ¿Para qué sirve el stack de un proceso?
El stack (pila) guarda:

Variables temporales y datos de funciones.
Direcciones para volver al punto correcto cuando una función termina.
### 4. ¿Para qué sirve el heap de un proceso?
El heap (montículo) es para guardar datos que necesitas en el programa por más tiempo y que pueden cambiar de tamaño. Usas funciones como malloc() para pedir memoria y free() para liberarla.

### 5. ¿Qué es la zona de texto de un proceso?
Es donde se guarda el código que ejecuta tu programa (las instrucciones). Está protegido para que no se modifique mientras el programa corre.

### 6. ¿Dónde se almacenan las variables globales inicializadas?
Las variables globales que tienen un valor asignado desde el inicio se guardan en la sección de datos.

### 7. ¿Dónde se almacenan las variables globales no inicializadas?
Las variables globales que no tienen un valor asignado al inicio se guardan en la sección BSS y se inicializan a cero automáticamente.

### 8. ¿Cuáles son los posibles estados de un proceso en general?
Los estados comunes de un proceso son:

- Nuevo: Se está creando.
- Listo: Esperando su turno para usar la CPU.
- Ejecutando: Usando la CPU.
- Bloqueado: Esperando algo (como entrada/salida).
- Terminado: Ha terminado y está listo para ser eliminado.
- 
# Evaluación Unidad 4
## Documentacion 

![image](https://github.com/user-attachments/assets/ddf1cf40-3cd0-4f1c-8328-18bff9c4545f)


no sabia como comenzar y por recomendacion me dicen que utilice el SDL_mutex para controlar el audio y cuanod sonaba 
luego se agrego el lock 

![image](https://github.com/user-attachments/assets/25ae33e9-8773-414f-87d7-bf4a5fbd3b28)

para asegurar de que los audios no saonaran uno sobre el otro

aqui ya se agrega el sonido de incioi del juego 

![image](https://github.com/user-attachments/assets/181e1ab2-c632-42a4-9678-95a91fdb04ce)

se agrega el sonido del movimeinto de player 

![image](https://github.com/user-attachments/assets/35b77241-8c2f-4333-b81c-af736db8b74f)

cuando se pierde 

![image](https://github.com/user-attachments/assets/f8fd92e8-c9f0-4ebc-8a31-30daf6f602ac)

aquie agregue un power up que e sun triangulo que aumenta la velocidad del juegador y geenra un sonido cunado el jugador lo toca 

![image](https://github.com/user-attachments/assets/b02b28f7-36eb-4c1f-96d8-eb5f576b7abb)

se agrega un sonido que es cunaod el jugador choca con los limites de la pantalla 

![image](https://github.com/user-attachments/assets/663e5da8-bce5-4a31-8c60-55b7af9ae779)


```c
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MAX_CIRCLES 25

typedef struct {
    int x, y;
    int radius;
    int vel_y;
    bool active;
} Circle;

typedef struct {
    int x, y;
    int width, height;
    int vel_x;
} Player;

typedef struct {
    int x, y;
    int size;
    int vel_y;
    bool active;
} Triangle;

bool game_over = false;
Player player;
Circle circles[MAX_CIRCLES];
Triangle triangle;
int active_circles = 1;
Uint32 last_add_time = 0;
Uint32 speed_boost_end_time = 0;
bool speed_boost_active = false;

SDL_mutex* mutex_audio;
SDL_Thread* audio_thread;

SDL_AudioDeviceID device_id;
SDL_AudioSpec wav_spec;
Uint8* wav_buffer;
Uint32 wav_length;

void initialize_player(Player* player) {
    player->x = WINDOW_WIDTH / 2;
    player->y = WINDOW_HEIGHT - 50;
    player->width = 40;
    player->height = 40;
    player->vel_x = 0;
}

void initialize_circle(Circle* circle) {
    circle->x = rand() % (WINDOW_WIDTH - circle->radius * 2) + circle->radius;
    circle->y = 0;
    circle->vel_y = 4 + rand() % 3;
    circle->radius = 20 + rand() % 30;
    circle->active = true;
}

void initialize_triangle(Triangle* triangle) {
    triangle->x = rand() % (WINDOW_WIDTH - 30) + 15;
    triangle->y = 0;
    triangle->size = 30;
    triangle->vel_y = 3;
    triangle->active = true;
}

void draw_circle(SDL_Renderer* renderer, int center_x, int center_y, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, center_x + dx, center_y + dy);
            }
        }
    }
}

void draw_triangle(SDL_Renderer* renderer, Triangle* triangle) {
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_Point points[4] = {
        {triangle->x, triangle->y},
        {triangle->x - triangle->size / 2, triangle->y + triangle->size},
        {triangle->x + triangle->size / 2, triangle->y + triangle->size},
        {triangle->x, triangle->y}
    };
    SDL_RenderDrawLines(renderer, points, 4);
}

bool detect_collision(Player* player, Circle* circle) {
    int dist_x = player->x + player->width / 2 - circle->x;
    int dist_y = player->y + player->height / 2 - circle->y;
    int distance = dist_x * dist_x + dist_y * dist_y;
    int radius_squared = circle->radius * circle->radius;

    return distance < radius_squared;
}

bool detect_collision_triangle(Player* player, Triangle* triangle) {
    return (player->x < triangle->x + triangle->size / 2 &&
        player->x + player->width > triangle->x - triangle->size / 2 &&
        player->y < triangle->y + triangle->size &&
        player->y + player->height > triangle->y);
}

void increase_difficulty(Uint32 current_time) {
    if ((current_time - last_add_time) > 5000 && active_circles < MAX_CIRCLES) {
        initialize_circle(&circles[active_circles]);
        active_circles++;
        last_add_time = current_time;
    }
}

void audio_callback(void* data) {
    while (!game_over) {
        SDL_LockMutex(mutex_audio);

        if (game_over) break;

        SDL_QueueAudio(device_id, wav_buffer, wav_length);
        SDL_PauseAudioDevice(device_id, 0);

        SDL_UnlockMutex(mutex_audio);
    }
}

void play_sound(const char* sound_path) {
    if (SDL_LoadWAV(sound_path, &wav_spec, &wav_buffer, &wav_length) == NULL) {
        printf("Error loading WAV: %s\n", SDL_GetError());
        return;
    }

    SDL_LockMutex(mutex_audio);

    SDL_QueueAudio(device_id, wav_buffer, wav_length);
    SDL_PauseAudioDevice(device_id, 0);

    SDL_UnlockMutex(mutex_audio); 
}

void play_collision_sound() {
    play_sound("collision_sound.wav");
}

int main(int argc, char* args[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "Error al inicializar SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_AudioSpec desired_spec;
    desired_spec.freq = 44100;
    desired_spec.format = AUDIO_S16SYS;
    desired_spec.channels = 2;
    desired_spec.samples = 4096;
    desired_spec.callback = NULL;

    device_id = SDL_OpenAudioDevice(NULL, 0, &desired_spec, &wav_spec, SDL_AUDIO_ALLOW_ANY_CHANGE);

    if (device_id == 0) {
        fprintf(stderr, "Error opening audio device: %s\n", SDL_GetError());
        return 1;
    }

    mutex_audio = SDL_CreateMutex();

    audio_thread = SDL_CreateThread(audio_callback, "AudioThread", NULL);

    SDL_Window* window = SDL_CreateWindow("Juego de esquivar", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    srand(time(0));
    initialize_player(&player);
    initialize_circle(&circles[0]);
    initialize_triangle(&triangle);

    last_add_time = SDL_GetTicks();
    bool game_running = true;
    SDL_Event event;

    play_sound("start_sound.wav");

    while (game_running && !game_over) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                game_running = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_a || event.key.keysym.sym == SDLK_d) {
                    play_sound("move_sound.wav");
                }
                if (event.key.keysym.sym == SDLK_a) player.vel_x = speed_boost_active ? -10 : -5;
                else if (event.key.keysym.sym == SDLK_d) player.vel_x = speed_boost_active ? 10 : 5;
            }
            else if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_a || event.key.keysym.sym == SDLK_d) player.vel_x = 0;
            }
        }

        player.x += player.vel_x;

        if (player.x < 0) {
            player.x = 0;
            play_collision_sound(); 
        }
        if (player.x + player.width > WINDOW_WIDTH) {
            player.x = WINDOW_WIDTH - player.width;
            play_collision_sound(); 
        }

        for (int i = 0; i < active_circles; i++) {
            if (circles[i].active) {
                circles[i].y += circles[i].vel_y;
                if (circles[i].y - circles[i].radius > WINDOW_HEIGHT) initialize_circle(&circles[i]);
                if (detect_collision(&player, &circles[i])) {
                    game_over = true;
                    play_sound("lose_sound.wav"); 
                }
            }
        }

        triangle.y += triangle.vel_y;
        if (triangle.y > WINDOW_HEIGHT) initialize_triangle(&triangle);
        if (detect_collision_triangle(&player, &triangle)) {
            speed_boost_active = true;
            speed_boost_end_time = SDL_GetTicks() + 5000; 
            play_sound("speed_boost_sound.wav"); 
        }

        if (speed_boost_active && SDL_GetTicks() > speed_boost_end_time) {
            speed_boost_active = false;
        }

        Uint32 current_time = SDL_GetTicks();
        increase_difficulty(current_time);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw everything
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect player_rect = { player.x, player.y, player.width, player.height };
        SDL_RenderFillRect(renderer, &player_rect);

        for (int i = 0; i < active_circles; i++) {
            if (circles[i].active) {
                draw_circle(renderer, circles[i].x, circles[i].y, circles[i].radius);
            }
        }

        draw_triangle(renderer, &triangle);

        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_CloseAudioDevice(device_id);
    SDL_Quit();

    return 0;
}

```
## Hilos 
se utilizan hilos y mutexes para manejar la sincronización y la ejecución concurrente de tareas. El uso de hilos en este caso se centra en la reproducción de audio, mientras que las otras tareas del juego, como la lógica del movimiento de los objetos y la detección de colisiones, se realizan de forma secuencial en el hilo principal.

Hilos en la Aplicación
1. Hilo de Audio:
El hilo de audio es creado con la función SDL_CreateThread, que se ejecuta en paralelo al hilo principal. Este hilo se encarga de gestionar la reproducción de audio durante el juego, asegurándose de que los sonidos se reproduzcan sin bloquear el hilo principal, lo que permite que el juego siga funcionando de manera fluida.

Función: La función audio_callback es la que se ejecuta en el hilo de audio. Esta función entra en un bucle donde verifica constantemente si el juego ha terminado (a través de la variable global game_over). Mientras el juego esté en ejecución, se encarga de reproducir los sonidos utilizando SDL_QueueAudio y SDL_PauseAudioDevice.
Sincronización con el hilo principal: El hilo de audio está sincronizado con el hilo principal mediante el uso de un mutex (un mecanismo de sincronización) para controlar el acceso a los recursos compartidos, en este caso, la reproducción de audio.
2. Hilo Principal:
El hilo principal gestiona la lógica del juego, que incluye el manejo de eventos, el movimiento de objetos, la detección de colisiones y el renderizado. En particular:

Eventos: Captura los eventos del teclado para mover al jugador o realizar otras interacciones.
Lógica de juego: Actualiza las posiciones de los objetos en el juego (jugador, círculos y triángulo), comprueba las colisiones, incrementa la dificultad y controla el estado del juego (si el jugador ha perdido o si el juego sigue corriendo).
Renderizado: Dibuja todos los objetos en la pantalla en cada fotograma y actualiza la visualización.
Sincronización de Hilos mediante Mutex
Para evitar que el acceso a recursos compartidos (en este caso, la reproducción de audio) cause problemas de concurrencia, se utiliza un mutex (mutex_audio). Este mutex asegura que solo un hilo pueda acceder a la cola de audio a la vez.

Cómo funciona la sincronización:
Bloqueo del mutex:

En la función play_sound, el hilo principal solicita acceso al mutex antes de reproducir cualquier sonido con SDL_LockMutex(mutex_audio). Esto asegura que el hilo de audio no esté reproduciendo un sonido en ese momento.
Reproducción de audio:

Luego, el sonido se coloca en la cola de audio con SDL_QueueAudio, y se inicia la reproducción con SDL_PauseAudioDevice(device_id, 0).
Desbloqueo del mutex:

Después de que el sonido se ha colocado en la cola y se ha iniciado, el mutex se desbloquea con SDL_UnlockMutex(mutex_audio). Esto permite que el hilo de audio acceda nuevamente al recurso y continúe reproduciendo otros sonidos.
De esta manera, se evita que los hilos accedan simultáneamente a los recursos de audio, lo que podría generar inconsistencias o colisiones de audio. El mutex actúa como un "candado" para garantizar que solo un hilo pueda realizar operaciones en el recurso compartido a la vez.

Por qué es necesario el uso de hilos y mutexes:
En aplicaciones de videojuegos, especialmente aquellas que manejan audio y gráficos, es común que el hilo principal de juego se vea bloqueado por tareas como la reproducción de sonidos o la carga de archivos. Si no se gestionaran adecuadamente, estas tareas bloquearían la ejecución del juego, causando una experiencia de usuario pobre. Al delegar la reproducción de audio a un hilo separado, se asegura que el juego continúe sin interrupciones.

Resumen:
El hilo principal gestiona la lógica del juego y el renderizado.
El hilo de audio maneja la reproducción de sonidos en segundo plano.
Se utilizan mutexes (mutex_audio) para sincronizar el acceso a los recursos de audio, garantizando que no haya interferencias entre el hilo principal y el hilo de audio.
Este enfoque permite una experiencia de usuario más fluida, evitando bloqueos y permitiendo que el juego y el audio se gestionen de forma concurrente.
