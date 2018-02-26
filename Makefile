CC=gcc
CFLAGS=-g -Wall -Wextra

BIN=cdippy
OBJS=adjudicator.o map.o territories.o main.o

$(BIN): $(addprefix src/,$(OBJS))
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
