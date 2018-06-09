#include <stdlib.h>
#include <string.h>

#include "memory-handling.h"
#include "data-structures.h"
#include "global-variables.h"
#include "symbols.h"

#define max_stack_references 1024
object* stack_references[max_stack_references];
int stack_reference_count = 0;

char max_stack_references_reached(void) {
	return stack_reference_count == max_stack_references;
}

void add_stack_reference(object* obj, object* reference) {
	if (max_stack_references_reached()) {
		fprintf(stderr, "too many stack references\n");
		exit(0);
	}
	if (obj->location > location_heap) {
		fprintf(stderr, "stack reference at %s\n", location_name[obj->location]);
		exit(0);
	}
	else if ((obj->location == location_heap) && (reference->location == location_stack)) {
		stack_references[stack_reference_count] = obj;
		stack_reference_count++;
	}
}

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
	switch (obj->type) {
		case type_string: return sizeof(object) + sizeof(char) * (1 + string_length(obj));
		case type_vector: return sizeof(object) + sizeof(object*) * vector_length(obj);
		default: return sizeof(object);
	}
}

char* target_offset(object* obj, int direction) {
	return (char*)(obj + 1) - ((direction == 1) ? 0 : (object_size(obj)));
}

void move_object(object* to, object* from, int direction) {
	(*to) = (*from);
	to->location = location_heap;
	from->location = location_moved;
	from->data.forward_reference.ref = to;
	char* next_target = target_offset(to, direction);
	if (is_string(to)) {
		strcpy(next_target, string_value(to));
		to->data.string.value = next_target;
	}
	else if (is_vector(to)) {
		memcpy(next_target, vector_data(to), sizeof(object*) * vector_length(to));
		to->data.vector.data = (object**)next_target;
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

void traverse_symbol(target_space space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.symbol.name, location);
}

void traverse_integer(target_space space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.integer.digits, location);
}

void traverse_fraction(target_space space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.fraction.numerator, location);
	move_if_necessary(space, &obj->data.fraction.denominator, location);
}

void traverse_complex(target_space space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.complex.real_part, location);
	move_if_necessary(space, &obj->data.complex.imag_part, location);
}

void traverse_list(target_space space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.list.first, location);
	move_if_necessary(space, &obj->data.list.rest, location);
}

void traverse_stream(target_space space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.stream.first, location);
	move_if_necessary(space, &obj->data.stream.rest, location);
}

void traverse_vector(target_space space, object* obj, object_location location) {
	int i;
	for (i = 0; i < vector_length(obj); i++) {
		move_if_necessary(space, &obj->data.vector.data[i], location);
	}
}

void traverse_vector_iterator(target_space space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.vector_iterator.vector, location);
}

void traverse_struct_definition(target_space space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.struct_definition.name, location);
	move_if_necessary(space, &obj->data.struct_definition.fields, location);
}

void traverse_struct_instance(target_space space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.struct_instance.type, location);
	move_if_necessary(space, &obj->data.struct_instance.data, location);
}

void traverse_binding(target_space space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.binding.name, location);
	move_if_necessary(space, &obj->data.binding.value, location);
}

void traverse_function(target_space space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.function.parameters, location);
	move_if_necessary(space, &obj->data.function.environment, location);
	move_if_necessary(space, &obj->data.function.body, location);
}

void traverse_environment(target_space space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.environment.bindings, location);
}

void traverse_call(target_space space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.call.function, location);
	move_if_necessary(space, &obj->data.call.arguments, location);
	move_if_necessary(space, &obj->data.call.continuation, location);
}

void traverse_continuation(target_space space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.continuation.call, location);
}

void traverse_syntax_object(target_space space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.syntax_object.syntax, location);
	move_if_necessary(space, &obj->data.syntax_object.origin, location);
}

void traverse_internal_error(target_space space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.internal_error.trace, location);
	move_if_necessary(space, &obj->data.internal_error.message, location);
}

void traverse_delay(target_space space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.delay.value, location);
}

void traverse_nothing(target_space space, object* obj, object_location location) {
}

typedef void (traversal)(target_space space, object* obj, object_location location);

traversal* traversal_function(object* obj) {
	switch (obj->type) {
		case type_symbol: return &traverse_symbol;
		case type_integer: return &traverse_integer;
		case type_fraction: return &traverse_fraction;
		case type_complex: return &traverse_complex;
		case type_list: return &traverse_list;
		case type_stream: return &traverse_stream;
		case type_vector: return &traverse_vector;
		case type_vector_iterator: return &traverse_vector_iterator;
		case type_struct_definition: return &traverse_struct_definition;
		case type_struct_instance: return &traverse_struct_instance;
		case type_function: return &traverse_function;
		case type_binding: return &traverse_binding;
		case type_environment: return &traverse_environment;
		case type_call: return &traverse_call;
		case type_continuation: return &traverse_continuation;
		case type_syntax_object: return &traverse_syntax_object;
		case type_internal_error: return &traverse_internal_error;
		case type_delay: return &traverse_delay;
		default: return &traverse_nothing;
	}
}

void traverse_object(target_space space, object* obj, object_location location) {
	(*traversal_function(obj))(space, obj, location);
}

void clear_garbage(memory_space* memory, object** root, object_location location, char move_root) {
	char* next_free = memory->next_free;
	target_space space;
	space.direction = memory->direction;
	space.target = &next_free;
	char* next_traversed = next_free;
	
	if (move_root) {
		move_if_necessary(space, root, location);
	}
	else {
		traverse_object(space, *root, location);
	}
	
	while (next_free != next_traversed) {
		object* obj = (object*)next_traversed;
		traverse_object(space, obj, location);
		next_traversed += space.direction * object_size(obj);
	}
	memory->next_free = next_free;
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
	printf("heap is at %p\n", (void*)space->memory);
}

#define print_gc 0

void perform_gc(object** root) {
	object_location location;
	char resize = 0;
	char* old_memory;
	char is_major = heap_full(&main_memory_space);
	if (is_major) {
		int used_data = used_heap_data(&main_memory_space);
		location = location_heap;
		if (main_memory_space.fill_and_resize) {
			if (print_gc) printf("major resizing gc\n");
			resize = 1;
			old_memory = main_memory_space.memory;
			init_memory_space(&main_memory_space, main_memory_space.half_size);
		}
		else {
			if (print_gc) printf("major gc\n");
			reset_memory_space(&main_memory_space);
		}
		if (print_gc) printf("used heap data pre-gc: %i\n", used_data);
	}
	else {
		location = location_stack;
		int i;
		for (i = 0; i < stack_reference_count; i++) {
			clear_garbage(&main_memory_space, stack_references+i, location, 0);
			stack_references[i] = no_object();
		}
	}
	clear_garbage(&main_memory_space, &symbol_list, location, 1);
	clear_garbage(&main_memory_space, root, location, 1);
	if (resize) {
		free(old_memory);
	}
	if (is_major) {
		main_memory_space.fill_and_resize =  resize_at_next_major_gc(&main_memory_space);
		if (print_gc) printf("used heap data post-gc: %i\n", used_heap_data(&main_memory_space));
	}
	stack_reference_count = 0;
}

void init_memory_spaces() {
	init_memory_space(&main_memory_space, 1024 * 1024 * 4);
}
