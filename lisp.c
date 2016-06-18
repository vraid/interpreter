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
	
	object* environment = static_environment();
	
	if (argc == 2) {
		char* filename = argv[1];
		// environment = read_file;
	}
	else {
		environment = static_environment();
	}
	
	object read_args[1];
	init_list_1(read_args, environment);
	
	object read_call;
	init_call(&read_call, &repl_read_entry_proc, read_args, end_cont());
	top_call(&read_call);
	
	return 0;
}
