CC=gcc
CFLAGS = -W -Wall -Wno-unused-parameter -Wno-unused-variable -pedantic -Werror -ggdb -std=c11

all: prog

prog: main.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf prog
