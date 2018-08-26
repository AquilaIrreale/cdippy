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

noinst_LIBRARIES = libcdippy.a
libcdippy_a_SOURCES = src/adjudicator.c \
                      src/map.c \
                      src/territories.c

CLEANFILES = src/parser.h \
             src/parser.c \
             src/lexer.c

EXTRA_DIST = README.md testsuite cdippy.h
RUNTESTDEFAULTFLAGS = --all --tool cdippy CDIPPY=./cdippy \
                      --srcdir $$srcdir/testsuite
