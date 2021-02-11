#include "files.h"

#include <stdlib.h>
#include "data-structures.h"
#include "object-init.h"
#include "heap-memory.h"
#include "call.h"
#include "delist.h"

object* read_file(object* args, object* cont) {
	object* filename;
	delist_1(args, &filename);
	
	char* name = string_value(filename);
	FILE* f = fopen(name, "r");
	if (!f) {
		object* str = alloc_string("could not open file");
		object* ls = alloc_list_2(str, filename);
		return throw_error(cont, ls);
	}
	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	char* file_buffer = alloc_heap_memory(size+1);
	fseek(f, 0, SEEK_SET);
	fread(file_buffer, size, 1, f);
	fclose(f);

	object* obj = alloc_obj();
	init_object(location_stack, type_string, obj);
	obj->data.string.value = file_buffer;
	obj->data.string.length = size;
	
	return call_cont(cont, obj);
}

void init_file_procedures(void) {
	init_primitive(&read_file, &read_file_proc);
}
