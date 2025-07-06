bin/anvil.out: $(wildcard src/*.c src/anvil/*.c)
	$(CC) -o $@ -w \
	    -lGL -lm -lX11 -lassimp -Iextern \
	    $^
