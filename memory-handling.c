#include <stdlib.h>
#include <string.h>

#include "memory-handling.h"
#include "data-structures.h"
#include "global-variables.h"
#include "symbols.h"

typedef struct {
	int direction;
	char** target;
} target_space;

typedef struct {
	int size;
	int half_size;
	int quarter_size;
	int direction;
	char fill_and_resize;
	char* memory;
	char* next_free;
} memory_space;

memory_space main_memory_space;

size_t object_size(object* obj) {
	if (is_string(obj)) {
		return sizeof(object) + sizeof(char) * string_length(obj);
	}
	else {
		return sizeof(object);
	}
}

void move_object(object* to, object* from, int direction) {
	(*to) = (*from);
	to->location = location_heap;
	from->location = location_moved;
	from->data.forward_reference.ref = to;
	if (is_string(to)) {
		char* next_target = (char*)(to + direction);
		memcpy(next_target, string_value(to), string_length(to));
		to->data.string.value = next_target;
		printf("string value: %s\n", next_target);
	}
}

void move_if_necessary(target_space space, object** obj, object_location location) {
	if ((**obj).location <= location) {
		move_object((object*)*(space.target), *obj, space.direction);
		*(space.target) += space.direction * object_size((**obj).data.forward_reference.ref);
	}
	
	// update the external pointer
	if ((**obj).location == location_moved) {
		(*obj) = (**obj).data.forward_reference.ref;
	}
}

void traverse_symbol(target_space space, object* object, object_location location) {
	move_if_necessary(space, &object->data.symbol.name, location);
}

void traverse_list(target_space space, object* object, object_location location) {
	move_if_necessary(space, &object->data.list.first, location);
	move_if_necessary(space, &object->data.list.rest, location);
}

void traverse_binding(target_space space, object* object, object_location location) {
	move_if_necessary(space, &object->data.binding.name, location);
	move_if_necessary(space, &object->data.binding.value, location);
}

void traverse_function(target_space space, object* object, object_location location) {
	move_if_necessary(space, &object->data.function.parameters, location);
	move_if_necessary(space, &object->data.function.environment, location);
	move_if_necessary(space, &object->data.function.body, location);
}

void traverse_environment(target_space space, object* object, object_location location) {
	move_if_necessary(space, &object->data.environment.bindings, location);
}

void traverse_call(target_space space, object* object, object_location location) {
	move_if_necessary(space, &object->data.call.function, location);
	move_if_necessary(space, &object->data.call.arguments, location);
	move_if_necessary(space, &object->data.call.continuation, location);
}

void traverse_continuation(target_space space, object* object, object_location location) {
	move_if_necessary(space, &object->data.continuation.call, location);
}

void traverse_object(target_space space, object* object, object_location location) {
	switch (object->type) {
		case type_symbol :
			traverse_symbol(space, object, location);
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
		case type_call :
			traverse_call(space, object, location);
			break;
		case type_continuation :
			traverse_continuation(space, object, location);
			break;
		case type_none :
		case type_boolean :
		case type_string :
		case type_primitive_procedure :
		case type_number :
		case type_file_port :
		default:
		break;
	}
}

void clear_garbage(char** to_space, object** root, int direction, object_location location) {
	char* next_free = *to_space;
	target_space space;
	space.direction = direction;
	space.target = &next_free;
	char* next_traversed = *to_space;
	
	move_if_necessary(space, root, location);
	
	while (next_free != next_traversed) {
		object* obj = (object*)next_traversed;
		traverse_object(space, obj, location);
		next_traversed += direction * object_size(obj);
	}
	*to_space = next_free;
}

int used_heap_data(memory_space* space) {
	int offset = space->next_free - space->memory;
	if (space->direction == 1) {
		return offset;
	}
	else {
		return space->size - offset;
	}
}

char heap_full(memory_space* space) {
	int max_size;
	if (space->fill_and_resize) {
		max_size = space->size;
	}
	else {
		max_size = space->half_size;
	}
	return used_heap_data(space) + max_stack_data > max_size;
}

char resize_at_next_major_gc(memory_space* space) {
	return used_heap_data(space) > space->quarter_size;
}

void reset_memory_space(memory_space* space) {
	space->next_free = space->memory + (space->half_size * (space->direction + 1));
	space->direction *= -1;
}

void init_memory_space(memory_space* space, int quarter_size) {
	space->quarter_size = quarter_size;
	space->half_size = 2 * quarter_size;
	space->size = 4 * quarter_size;
	space->direction = 1;
	space->fill_and_resize = 0;
	space->memory = malloc(space->size);
	space->next_free = space->memory;
	if (space->memory == 0) {
		printf("out of memory\n");
		exit(0);
	}
}

void perform_gc(object** root) {
	object_location location;
	char resize = 0;
	char* old_memory;
	char is_major = heap_full(&main_memory_space);
	printf("used heap data: %i\n", used_heap_data(&main_memory_space));
	if (is_major) {
		location = location_heap;
		if (main_memory_space.fill_and_resize) {
			printf("major resizing gc\n");
			resize = 1;
			old_memory = main_memory_space.memory;
			init_memory_space(&main_memory_space, main_memory_space.half_size);
		}
		else {
			printf("major gc\n");
			reset_memory_space(&main_memory_space);
		}
	}
	else {
		printf("minor gc\n");
		location = location_stack;
	}
	clear_garbage(&main_memory_space.next_free, &symbol_list, main_memory_space.direction, location);
	clear_garbage(&main_memory_space.next_free, root, main_memory_space.direction, location);
	if (resize) {
		free(old_memory);
	}
	if (is_major) {
		main_memory_space.fill_and_resize =  resize_at_next_major_gc(&main_memory_space);
	}
}

void init_memory_spaces() {
	init_memory_space(&main_memory_space, 1024 * 1024 * 4);
}
