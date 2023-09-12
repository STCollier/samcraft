CC ?= gcc
CFLAGS = -Wall -Wextra
BIN_DIR = ./bin

run:
	${CC} src/main.c lib/src/glad.c src/game/*.c src/world/*.c -Ilib $(CFLAGS) -o $(BIN_DIR)/main -ldl -lglfw -lm
	$(BIN_DIR)/main

debug:
	${CC} src/main.c lib/src/glad.c src/game/*.c src/world/*.c -Ilib $(CFLAGS) -o $(BIN_DIR)/main -ldl -lglfw -lm -g -Og -ggdb3
	valgrind $(BIN_DIR)/main --track-origins=yes