// Mandelbrot Zoomer (cpu)

#include <SDL2/SDL.h>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <cstdio>

static const int WIDTH  = 300;
static const int HEIGHT = 300;
static const int MAX_ITER = 200;

// Static auto-zoom target. Pick one "interesting" point on the boundary
// of the Mandelbrot set and zoom continuously toward it every frame.
//
// Zoom towards "Misiurewicz point" - a point whose
// neighborhood keeps generating new self-similar spiral.
static const double TARGET_X = -0.77568377;
static const double TARGET_Y =  0.13646737;

// Multiplicative zoom rate applied per frame (scale *= ZOOM_RATE).
// Smaller than 1.0 = zooming in. Tune for desired speed.
static const double ZOOM_RATE = 0.995;

static const double MIN_SCALE = 1e-13;

struct View {
    double centerX = TARGET_X;
    double centerY = TARGET_Y;
    double scale   = 3.0;
};

uint32_t iterToColor(double smoothIter, int maxIter) {
    if (smoothIter >= maxIter) {
        return 0x000000FF; // inside the set -> black
    }
    // Simple sinusoidal palette, cycling smoothly with iteration count.
    double t = smoothIter * 0.05;
    uint8_t r = static_cast<uint8_t>(std::clamp(0.5 + 0.5 * std::sin(t + 4.0), 0.0, 1.0) * 255.0);
    uint8_t g = 0;
    uint8_t b = static_cast<uint8_t>(std::clamp(0.5 + 0.5 * std::sin(t + 0.0), 0.0, 1.0) * 255.0);
    uint32_t pixel = (static_cast<uint32_t>(r) << 24) |
                      (static_cast<uint32_t>(g) << 16) |
                      (static_cast<uint32_t>(b) << 8)  |
                      0xFF;
    return pixel;
}

double mag(double x, double y) {
    return pow(x, 2) + pow(y, 2);
}
double computeSmoothIter(double cx, double cy, int maxIter) {
    // Complex version: Z_n+1 = Z_n **2 + C
    // Real version: (x=real, y=imag)
    double x=0, y=0;  // current point.
    int i = 0;
    for (; i < maxIter; ++i) {
        double xn = pow(x, 2) - pow(y, 2) + cx;
        double yn = 2*x*y + cy;
        x = xn;
        y = yn;
        if (pow(xn, 2) + pow(yn, 2) > 4.0) break; // |x,y|^2 > 4  <=>  |z| > 2
    }
    if (i == maxIter) return static_cast<double>(maxIter);

    // Smooth iteration count using the escaped magnitude.
    double logZn = std::log(mag(x, y)) / 2.0;
    double nu = std::log(logZn / std::log(2.0)) / std::log(2.0);
    return (i + 1) - nu;
}

void mandelbrotCell(uint32_t *pixels, double centerX, double centerY, double scale,
        int px, int py) {
    double aspect = static_cast<double>(WIDTH) / HEIGHT;
    double viewW = scale;
    double viewH = scale / aspect;

    double xMin = centerX - viewW / 2.0;
    double yMin = centerY - viewH / 2.0;

    double cy = yMin + (py / static_cast<double>(HEIGHT)) * viewH;
    double cx = xMin + (px / static_cast<double>(WIDTH)) * viewW;
    double smoothIter = computeSmoothIter(cx, cy, MAX_ITER);

    uint32_t pixel = iterToColor(smoothIter, MAX_ITER);

    pixels[py * WIDTH + px] = pixel;
}

void renderMandelbrot(std::vector<uint32_t>& pixels, const View& view) {
    for (int py = 0; py < HEIGHT; ++py) {
        for (int px = 0; px < WIDTH; ++px) {
            mandelbrotCell(pixels.data(), view.centerX, view.centerY, view.scale, px, py);
        }
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Mandelbrot (CPU, sequential)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT,
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

    std::vector<uint32_t> pixels(WIDTH * HEIGHT);

    View view;
    bool running = true;
    SDL_Event event;

    const int LOG_FRAMES = 60;
    int frames = LOG_FRAMES;
    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        running = false;
                    } else if (event.key.keysym.sym == SDLK_r) {
                        view = View{};
                    }
                    break;
            }
        }

        if (view.scale > MIN_SCALE) {
            // zoom
            view.centerX = TARGET_X;
            view.centerY = TARGET_Y;
            view.scale *= ZOOM_RATE;
        }

        frames --;
        int before, after;
        if (frames == 0) {
            before = SDL_GetPerformanceCounter();
        }
        renderMandelbrot(pixels, view);
        if (frames == 0) {
            after = SDL_GetPerformanceCounter();
            frames = LOG_FRAMES;
            float sec = float(after - before) / float(SDL_GetPerformanceFrequency());
            float fps = 1.0 / sec;
            printf("Last frame time: %f sec (%f fps)\n", sec, fps);
        }
        SDL_UpdateTexture(texture, nullptr, pixels.data(), WIDTH * sizeof(uint32_t));

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
