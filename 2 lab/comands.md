curl.exe -O https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
curl.exe -O https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h

g++ -std=c++20 -O2 main.cpp -o blur.exe

python -c "from PIL import Image; import random; w,h=1920,1080; data=bytes(random.randrange(256) for \_ in range(w*h*3)); Image.frombytes('RGB',(w,h),data).save('test.bmp')"

.\blur.exe test.bmp out1.bmp 1 10
.\blur.exe test.bmp out16.bmp 16 10

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

cmd /c "start /wait /b /affinity 1 .\build\blur.exe test.bmp out_x.bmp 4 10"
cmd /c "start /wait /b /affinity F .\build\blur.exe test.bmp out_x.bmp 4 10"

.\experiment.ps1

(Get-Content results.csv).Count
Get-Content results.csv -Head 8
