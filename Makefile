CC ?= gcc-13
CFLAGS = -Wall -Wextra -std=c2x
BIN_DIR = ./bin
CFLAGS = -I/opt/homebrew/include -I/opt/homebrew/include/freetype2
LDFLAGS = -L/opt/homebrew/lib/ -lglfw -llua -lm -lpthread -lfreetype

run:
	mkdir -p bin
	${CC} src/main.c lib/glad/glad.c lib/noise/osnoise.c -Ilib src/engine/*.c src/world/*.c -o ${BIN_DIR}/main ${CFLAGS} ${LDFLAGS} -O3
	${BIN_DIR}/main

debug:
	mkdir -p bin
	${CC} src/main.c lib/glad/glad.c lib/noise/osnoise.c -Ilib src/engine/*.c src/world/*.c -o ${BIN_DIR}/main ${CFLAGS} ${LDFLAGS} -g -fsanitize=address -fsanitize=undefined -Og
	${BIN_DIR}/main