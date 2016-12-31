#include "generic-arguments.h"

#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "symbols.h"

char argnames[generic_args_max][2];
object argcells[generic_args_max];

void init_generic_arguments(void) {
	int i;
	argnames[0][0] = 'a';
	argnames[0][1] = 0;
	generic_args[0] = make_static_symbol(argnames[0]);
	init_list_cell(&argcells[0], generic_args[0], empty_list());
	make_static(&argcells[0]);
	for (i = 1; i < generic_args_max; i++) {
		argnames[i][0] = argnames[i-1][0] + 1;
		argnames[i][1] = 0;
		generic_args[i] = make_static_symbol(argnames[i]);
		init_list_cell(&argcells[i], generic_args[i], &argcells[i-1]);
		make_static(&argcells[i]);
		generic_arg_list[i] = &argcells[i-1];
	}
	generic_arg_list[generic_args_max] = &argcells[generic_args_max]-1;
}
