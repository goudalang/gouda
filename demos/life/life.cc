// Game of Life (cpu, serial evaluation of cells)
// Cells outside of the screen evaluate to zero
// for the purposes of the step logic (no wrapping).
//
// To have produce an interesting simulation, we tile
// Gosper's Glider Gun across the top of the screen,
// which should emit gliders down and left.

#include <SDL2/SDL.h>
#include <cstdint>
#include <vector>
#include <cstdio>
using std::vector;

// Gosper's Glider Gun creates gliders, which decend down and to the left.
#define GUN_WIDTH  36
#define GUN_HEIGHT 9
static const int GOSPER_GUN[GUN_HEIGHT*GUN_WIDTH] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0, // row 0
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0, // row 1
    0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1, // row 2
    0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1, // row 3
    1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // row 4
    1,1,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0, // row 5
    0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0, // row 6
    0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // row 7
    0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // row 8
};

// PATTERN is used for initializing the grid. See setupGrid.
const int* PATTERN = GOSPER_GUN;
const int PATTERN_WIDTH = GUN_WIDTH;
const int PATTERN_HEIGHT = GUN_HEIGHT;
const int PATTERN_PADDING = 5;

static const int WIDTH  = 200;
static const int HEIGHT = 200;

// One cell is SCALExSCALE pixels on screen.
static const int SCALE = 4;

// RGBA8 color config
const uint32_t COLOR_ALIVE = 0xFFFF00FF;  // Yellow
const uint32_t COLOR_DEAD =  0x000000FF;  // Black

// Screen/state planning:
// We could be clever storing/packing state, but we have adequate
// memory and we'd still need to convert from our packed representation
// (1 bit per cell) to pixels (32 bits).
//
// With one pixel per cell (SCALE=1) and a 4k display (3840x2160),
// and 4 bytes per pixel:
// - 8294400 pixels
// - 33,177,600 bytes. (33M). even x2 for double buffering is acceptable.
void setupGrid(vector<uint32_t>* pixels) {
    // A randomly initialized grid is not very interesting, and typically reaches
    // equilibrium very quickly.
    // For that reason, we instead initialize the grid by repeatedly tiling some
    // pattern across the top edge of the screen. (this one-time setup loop
    // is slightly over-generalized in implementation to make it easy to
    // switch out different patterns and tiling)
    int x_start = PATTERN_PADDING, y_start = PATTERN_PADDING;
    while (x_start < WIDTH && y_start < HEIGHT) {
        // Copy pattern to x_start,y_start
        for (int dy = 0; dy < PATTERN_HEIGHT; dy++) {
            const int y = y_start + dy;
            if (y >= HEIGHT) break;
            for (int dx = 0; dx < PATTERN_WIDTH; dx++) {
                const int x = x_start + dx;
                if (x >= WIDTH) break;
                uint32_t color = (PATTERN[dy*PATTERN_WIDTH + dx])
                    ? COLOR_ALIVE
                    : COLOR_DEAD;
                (*pixels)[y*WIDTH + x] = color;
            }
        }
        // Tile along the x direction.
        x_start += PATTERN_WIDTH + PATTERN_PADDING;
    }
}

int countNeighbors(int x, int y, const vector<uint32_t> grid) {
    int living = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;  // exclude self.
            int ny = y + dy;
            int nx = x + dx;
            if (ny >= 0 && ny < HEIGHT &&
                nx >= 0 && nx < WIDTH) {
                living += grid[ny*WIDTH + nx] == COLOR_ALIVE;
            }
        }
    }
    return living;
}

// Computes one step of life. Pixels from previous generation are inputs
// to the next.
static void stepLife(
    const std::vector<uint32_t>& in_pixels,
    std::vector<uint32_t>* out_pixels) {

    for (int py = 0; py < HEIGHT; ++py) {
        for (int px = 0; px < WIDTH; ++px) {

            int neighbors = countNeighbors(px, py, in_pixels);
            const bool wasLive = in_pixels[py*WIDTH + px] == COLOR_ALIVE;
            bool alive = false;
            if (wasLive) {
                // underpopulation: <2 living neighbors dies
                // survival: 2-3 neighbors
                // overpopulation:  >3 live neighbors dies
                alive = neighbors >= 2 && neighbors <= 3;
            } else {
                // reproduction: exactly 3 neighbors: dead->live
                alive = neighbors == 3;
            }

            uint32_t pixel = alive ? COLOR_ALIVE : COLOR_DEAD;
            (*out_pixels)[py * WIDTH + px] = pixel;
        }
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Conway's Game of Life (CPU, sequential)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH*SCALE, HEIGHT*SCALE,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH, HEIGHT
    );

    // Life needs to read previous state while writing to the next.
    // We could be clever, but instead double-buffer, alternating which
    // buffer is the destination. This ensures the input is unmodified
    // for one step of computing Life.
    bool draw_to_a = true;
    std::vector<uint32_t> pixels_a(WIDTH * HEIGHT, COLOR_DEAD);
    std::vector<uint32_t> pixels_b(WIDTH * HEIGHT, COLOR_DEAD);
    setupGrid(&pixels_b);

    bool running = true;
    bool play = true;
    SDL_Event event;

    const int LOG_FRAMES = 60;
    int frames = LOG_FRAMES;

    const uint32_t* dst = pixels_b.data();
    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        running = false;
                    } else if (event.key.keysym.sym == SDLK_SPACE) {
                        play ^= 1;
                    }
                    break;
            }
        }

        if (play) {
            frames --;
            int before, after;
            if (frames == 0) {
                before = SDL_GetPerformanceCounter();
            }

            const uint32_t* dst = nullptr;
            if (draw_to_a) {
                stepLife(pixels_b, &pixels_a);
                dst = pixels_a.data();
            } else {
                stepLife(pixels_a, &pixels_b);
                dst = pixels_b.data();
            }
            draw_to_a = !draw_to_a;

            if (frames == 0) {
                after = SDL_GetPerformanceCounter();
                frames = LOG_FRAMES;
                float sec = float(after - before) / float(SDL_GetPerformanceFrequency());
                float fps = 1.0 / sec;
                float cells_per_sec = float(WIDTH*HEIGHT) / sec;
                printf("Last frame time: %f sec (%f fps) (%f cells/sec)\n", sec, fps, cells_per_sec);
            }
        }
        SDL_UpdateTexture(texture, nullptr, dst, WIDTH * sizeof(uint32_t));

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        SDL_Delay(1);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
