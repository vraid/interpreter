#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include "data-structures.h"
#include "standard-library.h"
#include "symbols.h"
#include "global-variables.h"
#include "call.h"
#include "allocation.h"
#include "object-init.h"
#include "print.h"
#include "repl-top.h"

object end_cont;
object end_proc;
object end_call;

object* end(object* args, object* cont) {
	return no_object();
}

int main(int argc, char** argv) {

	printf("running lisp interpreter. use ctrl-c to exit\n");
	
	init_type_names();
	init_symbols();
	init_global_variables();
	init_print_procedures();
	init_primitive_procedure(&end_proc, &end);
	init_call(&end_call, &end_proc, empty_list(), &end_cont);
	init_cont(&end_cont, &end_call);
	
	object* environment;
	
	if (argc == 2) {
		char* filename = argv[1];
		filename = filename;
		// environment = read_file;
	}
	else {
		environment = standard_environment();
	}	
	
	object call;
	object ls[1];
	init_list_1(ls, true());
	init_call(&call, &print_proc, ls, &end_cont);
	top_call(&call);
	
	repl_entry(environment);
	
	return 0;
}
