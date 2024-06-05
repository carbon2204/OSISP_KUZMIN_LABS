CC = gcc
CFLAGS = -W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic -D_XOPEN_SOURCE=700
LDFLAGS = -lpthread

SRC = main.c \
      queue/queue.c \
      consumer/consumer.c \
      message/message.c \
      producer/producer.c \
      functions/functions.c

OBJ = $(SRC:.c=.o)

all: program

program: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) program

run:
	$(MAKE) all
	./program
	$(MAKE) clean

.PHONY: all clean run