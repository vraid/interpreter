#include <stdlib.h>

#include "memory-handling.h"
#include "data-structures.h"

typedef struct {
	int direction;
	object** target;
} target_space;

void move_object(object* to, object* from) {
	(*to) = (*from);
	(*to).location = location_heap;
	(*from).location = location_moved;
	(*from).data.forward_reference.ref = to;
}

object* move_if_necessary(target_space space, object* object, object_location location) {
	if ((*object).location <= location) {
		move_object(*(space.target), object);
		*(space.target) += space.direction;
	}
	
	if ((*object).location == location_moved) {
		return (*object).data.forward_reference.ref;
	}
	else {
		return object;
	}
}

void traverse_number(target_space space, object* object, object_location location) {
}

void traverse_list(target_space space, object* object, object_location location) {
	(*object).data.list.first = move_if_necessary(space, (*object).data.list.first, location);
	(*object).data.list.rest = move_if_necessary(space, (*object).data.list.rest, location);
}

void traverse_binding(target_space space, object* object, object_location location) {
	(*object).data.binding.name = move_if_necessary(space, (*object).data.binding.name, location);
	(*object).data.binding.value = move_if_necessary(space, (*object).data.binding.value, location);
}

void traverse_function(target_space space, object* object, object_location location) {
	(*object).data.function.parameters = move_if_necessary(space, (*object).data.function.parameters, location);
	(*object).data.function.environment = move_if_necessary(space, (*object).data.function.environment, location);
	(*object).data.function.body = move_if_necessary(space, (*object).data.function.body, location);
}

void traverse_environment(target_space space, object* object, object_location location) {
	(*object).data.environment.bindings = move_if_necessary(space, (*object).data.environment.bindings, location);
}

void traverse_object(target_space space, object* object, object_location location) {
	switch ((*object).type) {
		case type_number :
			traverse_number(space, object, location);
			break;
		case type_list :
			traverse_list(space, object, location);
			break;
		case type_function :
			traverse_function(space, object, location);
			break;
		case type_binding :
			traverse_binding(space, object, location);
			break;
		case type_environment :
			traverse_environment(space, object, location);
			break;
		case type_none :
		case type_boolean :
		case type_symbol :
		case type_primitive_procedure :
		default:
		break;
	}
}

void clear_garbage(object* to_space, object* root, int direction, object_location location) {
	object* next_free = to_space;
	target_space space;
	space.direction = direction;
	space.target = &next_free;
	object* next_traversed = to_space;
	
	move_if_necessary(space, root, location);
	
	while (next_free != next_traversed) {
		traverse_object(space, next_traversed, location);
		next_traversed += direction;
	}
}
