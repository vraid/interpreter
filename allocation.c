#include "allocation.h"

object* allocate_object(void) {
	object* obj;
	
	obj = malloc(sizeof(object));
	if (obj == NULL) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}
	return obj;
}

object* allocate_object_type(object_type type) {
	object* obj = allocate_object();
	obj->type = type;
	return obj;
}

object* allocate_list_type(bracket_type type) {
	object* obj = allocate_object_type(type_list);
	obj->data.list.type = type;
	return obj;
}

object* allocate_list(void) {
	return allocate_list_type(round);
}
