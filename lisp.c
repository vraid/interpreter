#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include "data-structures.h"
#include "standard-library.h"
#include "generic-arguments.h"
#include "symbols.h"
#include "global-variables.h"
#include "environments.h"
#include "call.h"
#include "object-init.h"
#include "structs.h"
#include "sequences.h"
#include "higher-order.h"
#include "streams.h"
#include "strings.h"
#include "files.h"
#include "integers.h"
#include "fractions.h"
#include "complex.h"
#include "numbers.h"
#include "list-util.h"
#include "vectors.h"
#include "base-util.h"
#include "equivalence.h"
#include "memory-handling.h"
#include "workspace.h"
#include "syntax-base.h"
#include "syntax-sequence.h"
#include "syntax-validate.h"
#include "read.h"
#include "read-tables.h"
#include "eval.h"
#include "print.h"
#include "repl-top.h"

#include <unistd.h>
#include <limits.h>

int main(int argc, char** argv) {

	
	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd));
	char* path = argc < 2 ? cwd : argv[1];
	
	DIR* dir = opendir(path);
	if (dir)
	{
    	closedir(dir);
		printf("running lisp interpreter. use ctrl-c to exit\nworking directory: %s\n", path);
	}
	else if (ENOENT == errno)
	{
   	printf("not a directory: %s\n", path);
	}
	else
	{
   	/* opendir() failed for some other reason. */
	}
	
	init_data_structure_names();
	init_symbols();
	init_global_variables();
	init_struct_procedures();
	init_sequence_procedures();
	init_higher_order_procedures();
	init_stream_procedures();
	init_string_procedures();
	init_file_procedures();
	init_integer_procedures();
	init_fraction_procedures();
	init_complex_procedures();
	init_number_procedures();
	init_list_util_procedures();
	init_vector_procedures();
	init_base_util_procedures();
	init_read_procedures();
	init_read_table_procedures();
	init_print_procedures();
	init_eval_procedures();
	init_repl_procedures();
	init_environment_procedures();
	init_base_syntax_procedures();
	init_sequence_syntax_procedures();
	init_equivalence_procedures();
	init_memory_handling();
	init_memory_spaces(1024 * 1024 * 4);
	init_workspace();
	init_validate_procedures();
	init_generic_arguments();
	init_standard_functions();
	
	object* environment = static_environment();
	
	if (argc == 2) {
		char* filename = argv[1];
		// environment = read_file;
	}
	else {
		environment = static_environment();
	}
	
	object* read_args = alloc_list_1(environment);
	object* read_call = alloc_call(&repl_read_entry_proc, read_args, end_cont());
	object* read_table_call = alloc_call(&default_read_table_proc, empty_list(), alloc_cont(read_call));
	top_call(read_table_call);
	
	return 0;
}
