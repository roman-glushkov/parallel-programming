Remove-Item -Recurse -Force build  
cmake -B build  
cmake --build build
.\build\lab1.exe 100
