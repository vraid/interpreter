#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "data-structures.h"
#include "standard-library.h"
#include "global-variables.h"
#include "read.h"
#include "eval.h"
#include "print.h"

int main(void) {

	printf("running lisp interpreter. use ctrl-c to exit\n");
	
	init_global_variables();
	
	object* environment = standard_environment();
	object* ev;
	
	while(1) {
		printf("> ");
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