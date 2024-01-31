CC ?= gcc
CFLAGS = -Wall -Wextra -Wc2x-extensions
BIN_DIR = ./bin
CFLAGS = -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib/ -lglfw -llua -lm

run:
	${CC} src/main.c lib/glad/glad.c -Ilib src/engine/*.c src/world/*.c -o ${BIN_DIR}/main ${CFLAGS} ${LDFLAGS}
	${BIN_DIR}/main 