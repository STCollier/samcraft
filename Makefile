CC ?= gcc-13
CFLAGS = -Wall -Wextra
BIN_DIR = ./bin
CFLAGS = -I/opt/homebrew/include -I/opt/homebrew/include/freetype2
LDFLAGS = -L/opt/homebrew/lib/ -lglfw -llua -lm -lpthread -lfreetype

run:
	mkdir -p bin
	${CC} src/main.c lib/glad/glad.c lib/noise/osnoise.c -Ilib src/engine/*.c src/world/*.c -o ${BIN_DIR}/main ${CFLAGS} ${LDFLAGS} -O3 -std=c2x
	${BIN_DIR}/main

win:
	mkdir -p bin
	x86_64-w64-mingw32-gcc src/main.c lib/glad/glad.c lib/noise/osnoise.c -Ilib src/engine/*.c src/world/*.c -o ${BIN_DIR}/main ${CFLAGS} -Llib/mingw -lglfw3 -lgdi -llua54 -lfreetype-6 -O3

debug:
	mkdir -p bin
	${CC} src/main.c lib/glad/glad.c lib/noise/osnoise.c -Ilib src/engine/*.c src/world/*.c -o ${BIN_DIR}/main ${CFLAGS} ${LDFLAGS} -g -fsanitize=address -fsanitize=undefined -Og -std=c2x
	${BIN_DIR}/main