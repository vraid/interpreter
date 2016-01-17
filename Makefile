.PHONY: clean

lisp: *.c
	cc -Wall -Wextra -pedantic -std=gnu99 -o lisp.out *.c

clean:
	rm lisp.out
