mkdir build
cd build
cmake -S .. -B build -G "MinGW Makefiles"
cmake --build build
cd ..
glslc "shaders\shader.frag" -o "shaders\frag.spv"
glslc "shaders\shader.vert" -o "shaders\vert.spv"