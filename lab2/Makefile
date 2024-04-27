CC = gcc
CFLAGS_DEBUG = -W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic
CFLAGS_RELEASE = -W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic

.PHONY: clean all

MODE ?= debug

ifeq ($(MODE), debug)
	CFLAGS = $(CFLAGS_DEBUG)
else
	CFLAGS = $(CFLAGS_RELEASE)
endif

all: parent child

parent: parent.c Makefile
	$(CC) $(CFLAGS) parent.c -o parent

child: child.c Makefile
	$(CC) $(CFLAGS) child.c -o child

clean:
	rm -f parent
	rm -f child