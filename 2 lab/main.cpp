// blur <input.bmp> <output.bmp> <threads> [radius]
// Вариант 2: вертикальные полосы.
// Ограничение ядер через переменную окружения BLUR_CORES (Windows).

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#ifdef _WIN32
#include <windows.h>
#endif
#include <cstdlib>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

// ---------- Изображение ----------
struct Pixel { uint8_t r, g, b; };

struct Image {
    int width = 0, height = 0;
    std::vector<uint8_t> data;   // RGB, size = width*height*3

    Pixel at(int x, int y) const {
        const uint8_t* p = &data[(y * width + x) * 3];
        return { p[0], p[1], p[2] };
    }
    void set(int x, int y, Pixel c) {
        uint8_t* p = &data[(y * width + x) * 3];
        p[0] = c.r; p[1] = c.g; p[2] = c.b;
    }
};

Image LoadBMP(const std::string& path) {
    int w, h, n;
    uint8_t* pixels = stbi_load(path.c_str(), &w, &h, &n, 3);
    if (!pixels) throw std::runtime_error("cannot read " + path);

    Image img;
    img.width = w;
    img.height = h;
    img.data.assign(pixels, pixels + (size_t)w * h * 3);
    stbi_image_free(pixels);
    return img;
}

void SaveBMP(const std::string& path, const Image& img) {
    if (!stbi_write_bmp(path.c_str(), img.width, img.height, 3, img.data.data()))
        throw std::runtime_error("cannot write " + path);
}

// ---------- Работа ----------
struct Rect { int x0, y0, x1, y1; };

// Вариант 2: N вертикальных полос, каждая — на всю высоту.
std::vector<std::vector<Rect>> MakeWork(int w, int h, int n) {
    std::vector<std::vector<Rect>> work(n);
    for (int k = 0; k < n; ++k) {
        int x0 = k * w / n;
        int x1 = (k + 1) * w / n;
        if (x0 < x1)
            work[k].push_back({ x0, 0, x1, h });
    }
    return work;
}

// ---------- Размытие одного прямоугольника ----------
void BlurRect(const Image& src, Image& dst, Rect rc, int radius) {
    const int W = src.width;
    const int H = src.height;

    for (int y = rc.y0; y < rc.y1; ++y) {
        for (int x = rc.x0; x < rc.x1; ++x) {
            uint32_t sr = 0, sg = 0, sb = 0, cnt = 0;

            for (int dy = -radius; dy <= radius; ++dy) {
                int sy = std::clamp(y + dy, 0, H - 1);
                for (int dx = -radius; dx <= radius; ++dx) {
                    int sx = std::clamp(x + dx, 0, W - 1);
                    Pixel p = src.at(sx, sy);
                    sr += p.r; sg += p.g; sb += p.b; ++cnt;
                }
            }
            dst.set(x, y, { (uint8_t)(sr/cnt), (uint8_t)(sg/cnt), (uint8_t)(sb/cnt) });
        }
    }
}

// ---------- main ----------
int main(int argc, char** argv) {
#ifdef _WIN32
    if (const char* cores = std::getenv("BLUR_CORES")) {
        int n = std::atoi(cores);
        if (n > 0) {
            DWORD_PTR mask = ((DWORD_PTR)1 << n) - 1;
            SetProcessAffinityMask(GetCurrentProcess(), mask);
        }
    }
#endif

    if (argc < 4 || argc > 5) {
        std::cerr << "usage: blur <in.bmp> <out.bmp> <threads> [radius]\n";
        return 1;
    }

    int threads = 0, radius = 5;
    try {
        threads = std::stoi(argv[3]);
        if (argc == 5) radius = std::stoi(argv[4]);
    } catch (...) {
        std::cerr << "error: threads/radius must be integers\n";
        return 1;
    }
    if (threads < 1 || radius < 0) {
        std::cerr << "error: threads >= 1, radius >= 0\n";
        return 1;
    }

    try {
        Image src = LoadBMP(argv[1]);

        Image dst;
        dst.width  = src.width;
        dst.height = src.height;
        dst.data.assign(src.data.size(), 0);

        auto work = MakeWork(src.width, src.height, threads);

        auto t0 = std::chrono::steady_clock::now();
        {
            std::vector<std::jthread> pool;
            for (int k = 0; k < threads; ++k) {
                pool.emplace_back([&, k] {
                    for (const Rect& rc : work[k])
                        BlurRect(src, dst, rc, radius);
                });
            }
        }
        auto t1 = std::chrono::steady_clock::now();

        SaveBMP(argv[2], dst);

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        std::cout << threads << ", " << radius << ", "
                  << std::thread::hardware_concurrency() << ", " << ms << "\n";
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}