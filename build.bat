@echo off
python shader_compiler.py src/shaders
clang -g -O0 -Wextra -Werror -Isrc/extern src/*.c -o anvil.exe -lgdi32 -luser32 -lshell32 -lopengl32