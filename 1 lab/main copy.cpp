#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <algorithm> 
#include <chrono>     
#include <cstdint>     
#include <cstdlib>    
#include <iostream>    
#include <stdexcept>   
#include <string>
#include <thread>      
#include <vector>

// Представляет один пиксель RGB-изображения.
// Каждый канал цвета хранится в диапазоне 0..255.
struct Pixel 
{
    uint8_t r, g, b;
};

// Хранит изображение: его размеры и последовательность RGB-байтов.
// data имеет вид: R G B | R G B | R G B | ...
struct Image 
{
    int width = 0;
    int height = 0;
    std::vector<uint8_t> data;

    // Получает пиксель по координатам (x, y).
    // const означает, что функция только читает изображение и не изменяет его.
    Pixel at(int x, int y) const
    {
        // Находим начало нужного пикселя в одномерном массиве.
        // (y * width + x) — номер пикселя, * 3 — три RGB-канала.
        const uint8_t* p = &data[(y * width + x) * 3];

        return { p[0], p[1], p[2] };
    }

    // Записывает заданный цвет в пиксель с координатами (x, y).
    void set(int x, int y, Pixel c) 
    {
        uint8_t* p = &data[(y * width + x) * 3];

        p[0] = c.r;
        p[1] = c.g;
        p[2] = c.b;
    }
};


// Загружает изображение из файла и преобразует его в структуру Image.
Image LoadBMP(const std::string& path) 
{
    int w, h, n;

    // Загружаем изображение.
    // w — ширина, h — высота, n — исходное количество каналов.
    // Последний аргумент 3 заставляет библиотеку вернуть RGB-изображение
    // с тремя каналами независимо от исходного формата.
    uint8_t* pixels = stbi_load(path.c_str(), &w, &h, &n, 3);

    // Если изображение загрузить не удалось, выбрасываем исключение.
    if (!pixels) 
    {
        throw std::runtime_error("cannot read " + path);
    }

    Image img;
    img.width = w;
    img.height = h;

    // Копируем пиксели из временного буфера библиотеки
    // в собственный вектор изображения.
    img.data.assign(
        pixels,
        pixels + static_cast<size_t>(w) * h * 3
    );

    // Освобождаем временную память, выделенную stb_image.
    stbi_image_free(pixels);

    return img;
}


// Сохраняет изображение Image в BMP-файл.
void SaveBMP(const std::string& path, const Image& img) 
{
    // Передаём путь, размеры изображения, количество каналов RGB
    // и указатель на первый байт массива пикселей.
    if (!stbi_write_bmp(
        path.c_str(),
        img.width,
        img.height,
        3,
        img.data.data()))
    {
        throw std::runtime_error("cannot write " + path);
    }
}


// Прямоугольная область изображения.
// x0/y0 — начало области, x1/y1 — её границы.
struct Rect 
{
    int x0, y0, x1, y1;
};


// Разделяет изображение на вертикальные области
// и назначает их потокам.
// n — количество потоков.
std::vector<std::vector<Rect>> MakeWork(int w, int h, int n) 
{
    // Для каждого потока создаём свой список областей.
    std::vector<std::vector<Rect>> work(n);

    for (int k = 0; k < n; ++k) 
    {
        // Определяем левую и правую границы
        // вертикальной области для потока k.
        int x0 = k * w / n;
        int x1 = (k + 1) * w / n;

        // Если область имеет ненулевую ширину,
        // добавляем её в работу соответствующего потока.
        if (x0 < x1) 
        {
            work[k].push_back({ x0, 0, x1, h });
        }
    }

    return work;
}


// Выполняет размытие одной прямоугольной области изображения.
// src — исходное изображение, dst — изображение с результатом.
// rc — область, которую должен обработать текущий поток.
// radius — радиус области соседних пикселей.
void BlurRect(const Image& src, Image& dst, Rect rc, int radius) 
{
    const int W = src.width;
    const int H = src.height;

    // Перебираем все строки назначенной области.
    for (int y = rc.y0; y < rc.y1; ++y) 
    {
        // Перебираем все пиксели текущей строки.
        for (int x = rc.x0; x < rc.x1; ++x) 
        {
            // Накопители сумм RGB-каналов и количество
            // использованных соседних пикселей.
            uint32_t sr = 0, sg = 0, sb = 0, cnt = 0;

            // Перебираем вертикальное смещение
            // от -radius до +radius.
            for (int dy = -radius; dy <= radius; ++dy) 
            {
                // Получаем реальную координату соседнего пикселя.
                // std::clamp не позволяет выйти за границы изображения:
                // допустимый диапазон Y — от 0 до H - 1.
                int sy = std::clamp(y + dy, 0, H - 1);

                // Перебираем горизонтальное смещение
                // от -radius до +radius.
                for (int dx = -radius; dx <= radius; ++dx) 
                {
                    // Аналогично ограничиваем координату X.
                    // Допустимый диапазон — от 0 до W - 1.
                    int sx = std::clamp(x + dx, 0, W - 1);

                    // Получаем цвет соседнего пикселя из исходного изображения.
                    Pixel p = src.at(sx, sy);

                    // Суммируем значения RGB-каналов.
                    sr += p.r;
                    sg += p.g;
                    sb += p.b;

                    // Учитываем обработанный пиксель.
                    ++cnt;
                }
            }

            // Записываем в dst средний цвет всех пикселей
            // из области радиуса.
            dst.set(x, y, 
            {
                static_cast<uint8_t>(sr / cnt),
                static_cast<uint8_t>(sg / cnt),
                static_cast<uint8_t>(sb / cnt)
            });
        }
    }
}


int main(int argc, char** argv) 
{
#ifdef _WIN32

    // Если задана переменная окружения BLUR_CORES,
    // ограничиваем процесс указанным количеством
    // логических процессоров Windows.
    if (const char* cores = std::getenv("BLUR_CORES")) 
    {
        // Переводим значение переменной из строки в число.
        int n = std::atoi(cores);

        if (n > 0) 
        {
            // Формируем маску процессоров:
            // первые n битов будут установлены в 1.
            DWORD_PTR mask = (static_cast<DWORD_PTR>(1) << n) - 1;

            // Назначаем текущему процессу созданную маску.
            SetProcessAffinityMask(GetCurrentProcess(), mask);
        }
    }

#endif


    // Ожидаем:
    // argv[0] — имя программы
    // argv[1] — входной BMP
    // argv[2] — выходной BMP
    // argv[3] — количество потоков
    // argv[4] — необязательный радиус
    //
    // Поэтому argc должно быть 4 или 5.
    if (argc < 4 || argc > 5) 
    {
        std::cerr << "usage: blur <in.bmp> <out.bmp> <threads> [radius]\n";
        return 1;
    }


    int threads = 0;

    // Радиус по умолчанию — 5,
    // если пользователь не указал его отдельно.
    int radius = 5;


    try 
    {
        // argv[3] является строкой, поэтому преобразуем её в число.
        threads = std::stoi(argv[3]);

        // Если радиус был передан в командной строке,
        // преобразуем его из строки в число.
        if (argc == 5) 
        {
            radius = std::stoi(argv[4]);
        }
    } 
    catch (...) 
    {
        // Если введены нецелочисленные значения,
        // std::stoi выбрасывает исключение.
        std::cerr << "error: threads/radius must be integers\n";
        return 1;
    }


    // Количество потоков должно быть не меньше 1,
    // а радиус не может быть отрицательным.
    if (threads < 1 || radius < 0) 
    {
        std::cerr << "error: threads >= 1, radius >= 0\n";
        return 1;
    }


    try 
    {
        // Загружаем исходное изображение.
        Image src = LoadBMP(argv[1]);


        // Создаём изображение для результата.
        Image dst;

        // Размер результата совпадает с исходным изображением.
        dst.width = src.width;
        dst.height = src.height;

        // Выделяем память под все RGB-данные результата
        // и заполняем её нулями.
        dst.data.assign(src.data.size(), 0);


        // Разбиваем изображение на вертикальные области
        // между потоками.
        auto work = MakeWork(src.width, src.height, threads);


        // Фиксируем время начала параллельной обработки.
        // Загрузка изображения и сохранение результата
        // в это измерение не входят.
        auto t0 = std::chrono::steady_clock::now();


        {
            // Контейнер рабочих потоков.
            std::vector<std::jthread> pool;


            // Создаём указанное пользователем количество потоков.
            for (int k = 0; k < threads; ++k) 
            {
                // Каждый поток получает свой индекс k
                // и обрабатывает назначенную ему область.
                pool.emplace_back([&, k] 
                {
                    for (const Rect& rc : work[k]) 
                    {
                        // Выполняем размытие своей области изображения.
                        BlurRect(src, dst, rc, radius);
                    }
                });
            }
        }

        // После выхода из блока все jthread уничтожаются.
        // Их деструкторы автоматически дожидаются завершения потоков.
        // Поэтому здесь гарантированно завершена вся параллельная обработка.
        auto t1 = std::chrono::steady_clock::now();


        // Сохраняем обработанное изображение.
        SaveBMP(argv[2], dst);


        // Вычисляем продолжительность обработки
        // и переводим её в миллисекунды.
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            t1 - t0
        ).count();


        // Выводим параметры эксперимента:
        // количество потоков, радиус, доступное аппаратное
        // количество потоков и время обработки.
        std::cout << threads << ", "
                  << radius << ", "
                  << std::thread::hardware_concurrency() << ", "
                  << ms << "\n";
    } 
    catch (const std::exception& e) 
    {
        // Обрабатываем ошибки загрузки, сохранения и другие исключения.
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }


    // Успешное завершение программы.
    return 0;
}