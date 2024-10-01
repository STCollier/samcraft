CFLAGS := -Ilib -O3 -std=c2x -D_DEFAULT_SOURCE -Wall -Wextra
LDFLAGS := -lm -lpthread -lfreetype
SRC := src/main.c lib/glad/glad.c lib/lua/minilua.c lib/noise/osnoise.c src/engine/core/*.c src/engine/func/*.c src/engine/gfx/*.c src/engine/util/*.c src/world/*.c 

ifeq ($(OS), Windows_NT)
	CC ?= x86_64-w64-mingw32-gcc
	LDFLAGS += -lgdi -lglfw3 -Llib/mingw
else
	ifeq ($(shell uname -s), Darwin)
			CC ?= gcc
			CFLAGS += -I/opt/homebrew/include -I/opt/homebrew/include/freetype2 -I/usr/local/include/freetype2
			LDFLAGS += -L/opt/homebrew/lib/ -lglfw
	endif
	ifeq ($(shell uname -s), Linux)
			CC ?= gcc
			CFLAGS += -I/usr/include/freetype2
			LDFLAGS += -lglfw
	endif
endif

build:
	mkdir -p bin
	${CC} ${SRC} -o ./bin/main ${CFLAGS} ${LDFLAGS}

debug:
	mkdir -p bin
	${CC} ${SRC} -o ./bin/main ${CFLAGS} ${LDFLAGS} -g -fsanitize=address -fsanitize=undefined -Og 
	./bin/main
