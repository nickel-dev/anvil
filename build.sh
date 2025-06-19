#!/bin/sh
python shader_compiler.py src/shaders
mkdir -p bin/linux
gcc -g -O0 -Wextra -Werror -Iextern src/*.c -o bin/linux/anvil.out -lm -lGL -lX11