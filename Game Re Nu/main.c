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
