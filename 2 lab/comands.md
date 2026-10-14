# Программа на C++20, std::jthread, CMake

PS C:\PGTU\parallel programming\2 lab> cmake -B build -DCMAKE_BUILD_TYPE=Release
-- Configuring done (1.6s)
-- Generating done (0.1s)
-- Build files have been written to: C:/PGTU/parallel programming/2 lab/build

PS C:\PGTU\parallel programming\2 lab> cmake --build build --config Release
ninja: no work to do.

# Командная строка и обработка ошибок

PS C:\PGTU\parallel programming\2 lab> .\build\blur.exe
usage: blur <in.bmp> <out.bmp> <threads> [radius]

PS C:\PGTU\parallel programming\2 lab> .\build\blur.exe test.bmp
usage: blur <in.bmp> <out.bmp> <threads> [radius]

PS C:\PGTU\parallel programming\2 lab> .\build\blur.exe test.bmp out.bmp abc
error: threads/radius must be integers

PS C:\PGTU\parallel programming\2 lab> .\build\blur.exe test.bmp out.bmp 0
error: threads >= 1, radius >= 0

PS C:\PGTU\parallel programming\2 lab> .\build\blur.exe nonexistent.bmp out.bmp 4 10
error: cannot read nonexistent.bmp

# Результат не зависит от N — fc /b совпадает

PS C:\PGTU\parallel programming\2 lab> .\blur.exe test.bmp out1.bmp 1 10
1, 10, 4, 1779
PS C:\PGTU\parallel programming\2 lab> .\blur.exe test.bmp out16.bmp 16 10
16, 10, 4, 765
PS C:\PGTU\parallel programming\2 lab> .\blur.exe test.bmp out2.bmp 2 10
2, 10, 4, 1570
PS C:\PGTU\parallel programming\2 lab> .\blur.exe test.bmp out4.bmp 4 10
4, 10, 4, 1396
PS C:\PGTU\parallel programming\2 lab> .\blur.exe test.bmp out8.bmp 8 10
8, 10, 4, 1153

PS C:\PGTU\parallel programming\2 lab> fc.exe /b out1.bmp out16.bmp
Сравнение файлов out1.bmp и OUT16.BMP
FC: различия не найдены
PS C:\PGTU\parallel programming\2 lab> fc.exe /b out1.bmp out2.bmp
Сравнение файлов out1.bmp и OUT2.BMP
FC: различия не найдены
PS C:\PGTU\parallel programming\2 lab> fc.exe /b out1.bmp out4.bmp
Сравнение файлов out1.bmp и OUT4.BMP
FC: различия не найдены
PS C:\PGTU\parallel programming\2 lab> fc.exe /b out1.bmp out8.bmp
Сравнение файлов out1.bmp и OUT8.BMP
FC: различия не найдены

# Эксперимент: N=1..16, ядра 1..4, 5 повторов, медиана

.\experiment.ps1
[1/320] cores=1 threads=1 run=1 ... 1267 ms
[2/320] cores=1 threads=1 run=2 ... 1431 ms

PS C:\PGTU\parallel programming\2 lab> (Get-Content results.csv).Count
321

PS C:\PGTU\parallel programming\2 lab> Get-Content results.csv -Head 8
cores,threads,radius,hw,ms,run
1,1,10,1,1234,1
1,1,10,1,1348,2
1,1,10,1,1270,3
1,1,10,1,1266,4
1,1,10,1,1302,5
1,2,10,1,1437,1
1,2,10,1,1955,2
