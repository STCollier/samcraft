CC ?= gcc
CFLAGS = -Wall -Wextra
BIN_DIR = ./bin
LIBS = -ldl -lglfw -lm -lsqlite3 -I/usr/include/lua5.3/ -llua5.3

run:
	${CC} src/main.c lib/src/glad.c lib/noise/open-simplex-noise.c src/game/*.c src/world/*.c -Ilib $(CFLAGS) -o $(BIN_DIR)/main $(LIBS) -g -Og -ggdb3 -fsanitize=address
	$(BIN_DIR)/main 

debug:
	${CC} src/main.c lib/src/glad.c lib/noise/open-simplex-noise.c src/game/*.c src/world/*.c -Ilib $(CFLAGS) -o $(BIN_DIR)/main $(LIBS) -g -Og -ggdb3
	valgrind $(BIN_DIR)/main --track-origins=yes