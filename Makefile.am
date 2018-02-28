AM_CFLAGS = -Wall -Wextra
AM_YFLAGS = -d

BUILT_SOURCES = src/parser.h
noinst_HEADERS = src/parser.h

bin_PROGRAMS = cdippy
cdippy_SOURCES = src/adjudicator.c \
                 src/lexer.l \
                 src/main.c \
                 src/map.c \
                 src/parser.y \
                 src/territories.c

CLEANFILES = src/parser.h \
             src/parser.c \
             src/lexer.c
