// Mandelbrot Zoomer (parallel, gouda)

#include <cuda_runtime.h>
#include <SDL2/SDL.h>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <cstdio>

#define WIDTH  300
#define HEIGHT 300
#define MAX_ITER 200

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

__device__ uint iterToColor(float smoothIter) {
    if (smoothIter >= MAX_ITER) {
        return 0x000000FF; // inside the set -> black
    }
    // Simple sinusoidal palette, cycling smoothly with iteration count.
    float t = smoothIter * 0.05;
    uchar r = (uchar)(clamp(0.5 + 0.5 * sin(t + 4.0), 0.0, 1.0) * 255.0);
    uchar g = 0;
    uchar b = (uchar)(clamp(0.5 + 0.5 * sin(t + 0.0), 0.0, 1.0) * 255.0);
    uint32_t pixel = (((uint32_t)r) << 24) |
                     (((uint32_t)g) << 16) |
                     (((uint32_t)b) << 8)  |
                      0xFF;
    return pixel;
}

__device__ float mag(float x, float y) {
    return x*x + y*y;
}
__device__ float computeSmoothIter(float cx, float cy) {
    // Complex version: Z_n+1 = Z_n **2 + C
    // Real version: (x=real, y=imag)
    float x=0, y=0;  // current point.
    int i = 0;
    for (; i < MAX_ITER; ++i) {
        float xn = x*x - y*y + cx;
        float yn = 2*x*y + cy;
        x = xn;
        y = yn;
        if ((xn*xn + yn*yn) > 4.0) break; // |x,y|^2 > 4  <=>  |z| > 2
    }
    if (i == MAX_ITER) return (float)MAX_ITER;

    // Smooth iteration count using the escaped magnitude.
    float logZn = log(mag(x, y)) / 2.0;
    float nu = log(logZn / log(2.0)) / log(2.0);
    return (i + 1) - nu;
}

__global__ void mandelbrotCell(uint32_t *pixels, float centerX, float centerY, float scale) {
    int px = blockIdx.x * blockDim.x + threadIdx.x;
    int py = blockIdx.y * blockDim.y + threadIdx.y;
    if (px >= WIDTH || py >= HEIGHT) return;

    float aspect = (float)WIDTH / (float)HEIGHT;
    float viewW = scale;
    float viewH = scale / aspect;

    float xMin = centerX - viewW / 2.0;
    float yMin = centerY - viewH / 2.0;

    float cy = yMin + ((float)py / (float)HEIGHT) * viewH;
    float cx = xMin + ((float)px / (float)WIDTH) * viewW;
    float smoothIter = computeSmoothIter(cx, cy);

    uint32_t pixel = iterToColor(smoothIter);

    pixels[py * WIDTH + px] = pixel;
}


int div_ceil(int num, int denom) {
    return (num + denom - 1) / denom;
}

uint32_t *pixels_device = nullptr;
void renderMandelbrot(std::vector<uint32_t>& pixels, const View& view) {
    dim3 tile(16, 16, 1);
    dim3 tiles(
        div_ceil(WIDTH, 16),
        div_ceil(HEIGHT, 16),
        1
    );
    float cx = (float)view.centerX;
    float cy = (float)view.centerY;
    float scale = (float)view.scale;
    mandelbrotCell<<<tiles, tile>>>(pixels_device, cx, cy, scale);
    cudaDeviceSynchronize();

    cudaMemcpy(pixels.data(), pixels_device,
            sizeof(uint32_t) * WIDTH * HEIGHT,
            cudaMemcpyDeviceToHost);
    // for (int py = 0; py < HEIGHT; ++py) {
    //     for (int px = 0; px < WIDTH; ++px) {
    //         mandelbrotCell(pixels.data(), view.centerX, view.centerY, view.scale, px, py);
    //     }
    // }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    cudaMalloc(&pixels_device, sizeof(uint32_t)*WIDTH*HEIGHT);

    SDL_Window* window = SDL_CreateWindow(
        "Mandelbrot (Gouda)",
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

        SDL_Delay(16);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
