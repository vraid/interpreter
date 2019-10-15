#include "read-tables.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "call.h"
#include "read.h"

typedef struct {
	char* name;
	read_type t;
	object* proc;
} read_entry;

object* default_read_table(object* args, object* cont) {
	object* table = empty_list();
	read_entry entries[] = {
		{"t", read_type_atom, &read_true_proc},
		{"f", read_type_atom, &read_false_proc}};

	int length = sizeof(entries) / sizeof(entries[0]);
	for (int n = 0; n < length; n++) {
		read_entry entry = entries[n];
		object* name = alloc_string(entry.name);
		object* obj = alloc_reader_entry(entry.t, entry.proc);
		table = alloc_list_cell(alloc_list_2(name, obj), table);
	}
	return call_cont(cont, table);
}

void init_read_table_procedures(void) {
	init_primitive(&default_read_table, &default_read_table_proc);
}
