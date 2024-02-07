CC ?= gcc
CFLAGS = -Wall -Wextra -std=c2x 
BIN_DIR = ./bin
CFLAGS = -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib/ -lglfw -llua -lm

run:
	${CC} src/main.c lib/glad/glad.c -Ilib src/engine/*.c src/world/*.c -o ${BIN_DIR}/main ${CFLAGS} ${LDFLAGS}
	${BIN_DIR}/main

debug:
	${CC} src/main.c lib/glad/glad.c -Ilib src/engine/*.c src/world/*.c -o ${BIN_DIR}/main ${CFLAGS} ${LDFLAGS} -g -fsanitize=address
	${BIN_DIR}/main