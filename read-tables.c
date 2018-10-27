#include "read-tables.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "call.h"
#include "read.h"

object* default_read_table(object* args, object* cont) {
	object* table = empty_list();
	object* t = alloc_list_2(alloc_string("t"), alloc_reader_entry(read_type_atom, &read_true_proc));
	object* f = alloc_list_2(alloc_string("f"), alloc_reader_entry(read_type_atom, &read_false_proc));
	table = alloc_list_2(t, f);
	return call_cont(cont, table);
}

void init_read_table_procedures(void) {
	init_primitive(&default_read_table, &default_read_table_proc);
}
