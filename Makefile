.PHONY: clean

lisp: *.c
	cc -Wall -Wextra -pedantic -std=gnu99 -o lisp *.c

clean:
	rm lisp
