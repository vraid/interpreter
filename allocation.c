#include <stdlib.h>
#include <stdio.h>
#include "allocation.h"
#include "global-variables.h"

object* allocate(void) {
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

object* allocate_object(object_type type) {
	object* obj = allocate();
	init_object(location_stack, type, obj);
	return obj;
}

object* new_list(void) {
	object* obj = allocate_object(type_list);
	obj->data.list.type = shapeless;
	obj->data.list.first = no_object();
	obj->data.list.rest = empty_list();
	return obj;
}
