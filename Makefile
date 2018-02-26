CC=gcc
CFLAGS=-g -Wall -Wextra
#LFLAGS=-d

BIN=cdippy
OBJS=adjudicator.o map.o territories.o lexer.o parser.o main.o

.PHONY: clean all

all: $(BIN)

$(BIN): $(addprefix src/,$(OBJS))
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

src/parser.c src/parser.h: src/parser.y
	yacc -d src/parser.y
	mv y.tab.h src/parser.h
	mv y.tab.c src/parser.c

src/lexer.c: src/lexer.l
	lex $(LFLAGS) src/lexer.l
	mv lex.yy.c src/lexer.c

clean:
	rm -f $(BIN) src/*.o src/parser.c src/lexer.c
