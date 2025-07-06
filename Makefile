CC := clang
CFLAGS := -Wextra -g -O0
LIB := -lGL -lm -lX11 -lassimp
INC := -Iextern
SRC := src/*.c src/anvil/*.c
BIN := anvil

anvil: $(wildcard $(SRC))
	$(CC) -o$(BIN) $(SRC) $(LIB) $(INC)

force:
	$(CC) -o$(BIN) $(SRC) $(LIB) $(INC)
