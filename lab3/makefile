CC = gcc
CFLAGS = -W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic
.PHONY : clean

all : parent child run

parent: parent.c
	$(CC) $(CFLAGS) parent.c -o parent

child: child.c
	 $(CC) $(CFLAGS) child.c -o child

run: parent
	./parent

clean:
	rm -f parent child

