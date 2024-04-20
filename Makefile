CC ?= gcc
CFLAGS = -Wall -Wextra -std=c2x
BIN_DIR = ./bin
CFLAGS = -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib/ -lglfw -llua -lm -lpthread

run:
	mkdir -p bin
	${CC} src/main.c lib/glad/glad.c lib/noise/osnoise.c -Ilib src/engine/*.c src/world/*.c -o ${BIN_DIR}/main ${CFLAGS} ${LDFLAGS} -O3
	${BIN_DIR}/main

debug:
	${CC} src/main.c lib/glad/glad.c lib/noise/osnoise.c -Ilib src/engine/*.c src/world/*.c -o ${BIN_DIR}/main ${CFLAGS} ${LDFLAGS} -g -fsanitize=address -fsanitize=undefined -Og
	${BIN_DIR}/main