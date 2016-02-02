#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include "data-structures.h"
#include "standard-library.h"
#include "global-variables.h"
#include "read.h"
#include "eval.h"
#include "print.h"

int* stack_top;
jmp_buf buf;

int main(void) {

	printf("running lisp interpreter. use ctrl-c to exit\n");
	
	init_global_variables();
	
	object* environment = standard_environment();
	object* ev;
	
	setjmp(buf);
	int i = 0;
	stack_top = &i;	
	
	while (1) {
		printf("> ");
		// eval with print continuation, setting active environment
		ev = eval(environment, read(stdin));
		if (is_environment(ev)) {
			environment = ev;
		}
		else {
			write(ev);
			printf("\n");
		}
	}
	return 0;
}