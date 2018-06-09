CC = clang

CFLAGS  = -g -Wall -Wextra -pedantic

all: *.c ; $(CC) $(CFLAGS) -o lisp.out *.c

.PHONY: clean
clean: rm -f *.o lisp.out