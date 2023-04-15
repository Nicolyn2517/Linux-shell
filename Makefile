CC=gcc
CFLAGS=-g -pedantic -std=gnu17 -Wall -Werror -Wextra

.PHONY: all
all: nyush

nyush: nyush.o getpath.o redirect.o

nyush.o: nyush.c getpath.h redirect.h

getpath.o: getpath.c getpath.h

redirect.o: redirect.c redirect.h

.PHONY: clean
clean:
	rm -f *.o nyush
