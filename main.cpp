#include <SDL2/SDL.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <cstdio>
#include <cstdlib>
#include <png.h>

int sec(int code)
{
    if (code < 0) {
        fprintf(stderr, "SDL pooped itself: %s\n", SDL_GetError());
        abort();
    }
    return code;
}

template<typename T> T *sec(T *ptr) {
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
    {Tile::Wall, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Wall,  Tile::Wall,  Tile::Wall,  Tile::Wall,  Tile::Wall},
};

struct Sprite
{
    SDL_Rect srcrect;
    SDL_Texture *texture;
};

void render_sprite(SDL_Renderer *renderer,
                         Sprite texture,
                         SDL_Rect destrect)
{
    sec(SDL_RenderCopy(
        renderer,
        texture.texture,
        &texture.srcrect,
        &destrect
    ));
}

void render_level(SDL_Renderer *renderer, Sprite wall_texture)
{
    for (int y = 0; y < LEVEL_HEIGHT; ++y) {
        for (int x = 0; x < LEVEL_WIDTH; ++x) {
            switch (level[y][x]) {
                case Tile::Empty: {
                } break;
                case Tile::Wall: {
                    sec(SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255));
                    render_sprite(renderer, wall_texture,
                                    {x * TILE_SIZE, y * TILE_SIZE,
                                     TILE_SIZE, TILE_SIZE});

                } break;
            }
        }
    }
}

SDL_Texture *load_texture_from_png_file(SDL_Renderer *renderer, const char *image_filename)
{
    
    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;
    if (!png_image_begin_read_from_file(&image, image_filename)) {
        fprintf(stderr, "Could not read file `%s`:%s\n", image_filename, image.message);
        abort();
    }
    image.format = PNG_FORMAT_RGBA;
    printf("Width: %d, Height: %d\n", image.width, image.height);
    uint32_t *tileset_pixels = (uint32_t *)malloc(sizeof(uint32_t) * image.width * image.height);
    
    if (!png_image_finish_read(&image, nullptr, tileset_pixels, 0, nullptr)) {
        fprintf(stderr, "libpng pooped itself: %s\n", image.message);
    }

    SDL_Surface *image_surface = sec(SDL_CreateRGBSurfaceFrom(
        tileset_pixels,
        image.width,
        image.height, 
        32,
        image.width * 4,
        0x000000FF,
        0x0000FF00,
        0x00FF0000,
        0xFF000000
    ));

    SDL_Texture *image_texture = sec(SDL_CreateTextureFromSurface(renderer, image_surface));
    SDL_FreeSurface(image_surface);

    return image_texture;
}

int main(void)
{
    sec(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS));

    SDL_Window *window = sec(SDL_CreateWindow(
        "Something",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800,
        600,
        SDL_WINDOW_SHOWN
    ));

    SDL_Renderer *renderer = sec(SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    ));

    // TODO(#2): replace fantasy_tiles.png with our own assets
    SDL_Texture *tileset_texture = load_texture_from_png_file(renderer, "fantasy_tiles.png");
    Sprite wall_texture = {
        .srcrect = {120, 128, 16, 16},
        .texture = tileset_texture
    };

    SDL_Texture *walking_texture = load_texture_from_png_file(renderer, "walking-12px-zoom.png");
    constexpr int walking_frame_size = 48;
    constexpr int walking_frame_count = 4;
    constexpr int walking_frame_duration = 200;

    int walking_frame_current = 0;

    Sprite walking_frames[walking_frame_count];

    for (int i = 0; i < walking_frame_count; ++i) {
        walking_frames[i].srcrect = {
            .x = i * walking_frame_size,
            .y = 0,
            .w = walking_frame_size,
            .h = walking_frame_size
        };
        walking_frames[i].texture = walking_texture;
    }

    Uint32 walking_frame_cooldown = walking_frame_duration;

    int x = 0;
    const Uint8 *keyboard = SDL_GetKeyboardState(NULL);
    bool quit = false;

    while (!quit) {
        const Uint32 begin = SDL_GetTicks();
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: {
                    quit = true;
                }break;
            }
        }

        if (keyboard[SDL_SCANCODE_D]) {
            x += 1;
        } else if (keyboard[SDL_SCANCODE_A]) {
            x -= 1;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // draw here
        render_level(renderer, wall_texture);
        render_sprite(
                renderer,
                walking_frames[walking_frame_current], 
                { x, 4 * TILE_SIZE - walking_frame_size, walking_frame_size, walking_frame_size });


        SDL_RenderPresent(renderer);

        const Uint32 dt = SDL_GetTicks() - begin;
        if (dt < walking_frame_cooldown) {
            walking_frame_cooldown -= dt;
        } else {
            walking_frame_current = (walking_frame_current + 1)%walking_frame_count;
            walking_frame_cooldown = walking_frame_duration;
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
