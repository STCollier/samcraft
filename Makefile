CC ?= gcc-13
CFLAGS = -Wall -Wextra
BIN_DIR = ./bin
CFLAGS = -I/usr/include -I/usr/include/freetype2
LDFLAGS = -L/opt/homebrew/lib/ -lglfw -llua -lm -lpthread -lfreetype

.PHONY: clean

run: 
	build
	{BIN_DIR}/main
clean:
	rm -rf ${BIN_DIR}

build: 
	clean
	mkdir -p ${BIN_DIR}
	${CC} src/main.c lib/glad/glad.c lib/noise/osnoise.c -Ilib src/engine/core/*.c src/engine/func/*.c src/engine/gfx/*.c src/engine/util/*.c src/world/*.c -o ${BIN_DIR}/main ${CFLAGS} ${LDFLAGS} -O3 -std=c2x

win: CC=x86_64-w64-mingw32-gcc
win: clean run
debug: CFLAGS+= -g -fsanitize=address -fsanitize=undefined -Og
debug: clean run