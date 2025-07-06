#!/bin/sh

clang -lassimp -lGL -lm -lX11 src/*.c src/anvil/*.c -Iextern -g -O0 -w -obin/anvil.out
