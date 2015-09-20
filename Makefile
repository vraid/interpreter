.PHONY: clean

lisp: lisp.c
	cc -Wall -ansi -o lisp lisp.c

clean:
	rm lisp
