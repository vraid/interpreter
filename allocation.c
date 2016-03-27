#include <stdlib.h>
#include <stdio.h>
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

void init_object(object_location loc, object_type type, object* obj) {
	obj->location = loc;	
	obj->type = type;
}

object* allocate_object_type(object_type type) {
	object* obj = allocate_object();
	init_object(location_stack, type, obj);
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

object* allocate_object_boolean(char value) {
	object* obj;
	
	obj = allocate_object_type(type_boolean);
	obj->data.boolean.value = value;
	return obj;
}
