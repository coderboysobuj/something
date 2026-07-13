#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>

int sdl(int code)
{
    if (code < 0) {
        fprintf(stderr, "SDL pooped itself: %s\n", SDL_GetError());
        abort();
    }
    return code;
}

template<typename T> T *sdl(T *ptr) {
    if (ptr == nullptr) {
        fprintf(stderr, "SDL pooped itself: %s\n", SDL_GetError());
        abort();
    }
    return ptr;
}

constexpr int TILE_SIZE = 64;

constexpr int LEVEL_WIDTH = 5;
constexpr int LEVEL_HEIGHT = 5;

enum class Tile
{
    Empty = 0,
    Wall
};

Tile level[LEVEL_HEIGHT][LEVEL_WIDTH] = {
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Wall,  Tile::Wall,  Tile::Wall,  Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty}
};

void render_level(SDL_Renderer *renderer)
{
    for (int y = 0; y < LEVEL_HEIGHT; ++y) {
        for (int x = 0; x < LEVEL_WIDTH; ++x) {
            switch (level[y][x]) {
                case Tile::Empty: {
                } break;
                case Tile::Wall: {
                    SDL_Rect rect = {
                        x * TILE_SIZE,
                        y * TILE_SIZE,
                        TILE_SIZE,
                        TILE_SIZE
                    };

                    sdl(SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255));
                    sdl(SDL_RenderFillRect(renderer, &rect));

                } break;
            }
        }
    }
}



int main(void)
{
    sdl(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS));

    SDL_Window *window = sdl(SDL_CreateWindow(
        "Something",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800,
        600,
        SDL_WINDOW_SHOWN
    ));

    SDL_Renderer *renderer = sdl(SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    ));

    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // draw here
        render_level(renderer);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
