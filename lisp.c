#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "data-structures.h"
#include "standard-library.h"
#include "symbols.h"
#include "global-variables.h"
#include "environments.h"
#include "call.h"
#include "object-init.h"
#include "list-util.h"
#include "base-util.h"
#include "memory-handling.h"
#include "base-syntax.h"
#include "read.h"
#include "eval.h"
#include "print.h"
#include "repl-top.h"

object end_cont;
object end_proc;
object end_call;

object* end(object* args, object* cont) {
	printf("end reached\n");
	return no_object();
}

int main(int argc, char** argv) {

	printf("running lisp interpreter. use ctrl-c to exit\n");
	
	init_data_structure_names();
	init_symbols();
	init_global_variables();
	init_list_util_procedures();
	init_base_util_procedures();
	init_read_procedures();
	init_print_procedures();
	init_eval_procedures();
	init_repl_procedures();
	init_environment_procedures();
	init_standard_functions();	
	init_base_syntax_procedures();
	init_memory_spaces();
	
	init_primitive_procedure(&end_proc, &end);
	init_call(&end_call, &end_proc, empty_list(), &end_cont);
	init_cont(&end_cont, &end_call);
	
	object* environment = static_environment();
	
	if (argc == 2) {
		char* filename = argv[1];
		// environment = read_file;
	}
	else {
		environment = static_environment();
	}
	
	object ls[1];
	init_list_1(ls, environment);
	
	object call;
	init_call(&call, &repl_read_entry_proc, ls, &end_cont);
	top_call(&call);
	
	return 0;
}
