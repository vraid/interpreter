#include <stdlib.h>
#include <string.h>

#include "memory-handling.h"
#include "data-structures.h"
#include "object-init.h"
#include "global-variables.h"
#include "symbols.h"

object* mutation_references;

void add_mutation_reference(object* ls, object* obj, object* reference) {
	if (obj->location > location_heap) {
		fprintf(stderr, "stack reference at %s\n", location_name[obj->location]);
		exit(0);
	}
	else if ((obj->location == location_heap) && (reference->location == location_stack)) {
		mutation_references = init_list_cell(ls, obj, mutation_references);
	}
}

object* malloc_references;

void add_malloc_reference(object* ls, object* obj, long size, char* reference) {
	malloc_references = init_list_cell(ls, init_memory_reference(obj, size, reference), malloc_references);
}

typedef struct {
	int direction;
	char** target;
} target_space;

typedef struct {
	long size;
	long half_size;
	long quarter_size;
	int direction;
	char fill;
	char* memory;
	char* next_free;
} memory_space;

memory_space main_memory_space;

int object_size(object* obj) {
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

void move_if_necessary(target_space* space, object** obj, object_location location) {
	if ((**obj).location <= location) {
		move_object((object*)*(space->target), *obj, space->direction);
		*(space->target) += space->direction * object_size(*obj);
	}
	
	// update the external pointer
	if ((**obj).location == location_moved) {
		(*obj) = (**obj).data.forward_reference.ref;
	}
}

void traverse_symbol(target_space* space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.symbol.name, location);
}

void traverse_integer(target_space* space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.integer.digits, location);
}

void traverse_fraction(target_space* space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.fraction.numerator, location);
	move_if_necessary(space, &obj->data.fraction.denominator, location);
}

void traverse_complex(target_space* space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.complex.real_part, location);
	move_if_necessary(space, &obj->data.complex.imag_part, location);
}

void traverse_list(target_space* space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.list.first, location);
	move_if_necessary(space, &obj->data.list.rest, location);
}

void traverse_stream(target_space* space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.stream.first, location);
	move_if_necessary(space, &obj->data.stream.rest, location);
}

void traverse_vector(target_space* space, object* obj, object_location location) {
	int i;
	for (i = 0; i < vector_length(obj); i++) {
		move_if_necessary(space, &obj->data.vector.data[i], location);
	}
}

void traverse_vector_iterator(target_space* space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.vector_iterator.vector, location);
}

void traverse_struct_definition(target_space* space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.struct_definition.name, location);
	move_if_necessary(space, &obj->data.struct_definition.fields, location);
}

void traverse_struct_instance(target_space* space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.struct_instance.type, location);
	move_if_necessary(space, &obj->data.struct_instance.data, location);
}

void traverse_binding(target_space* space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.binding.name, location);
	move_if_necessary(space, &obj->data.binding.value, location);
}

void traverse_function(target_space* space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.function.parameters, location);
	move_if_necessary(space, &obj->data.function.environment, location);
	move_if_necessary(space, &obj->data.function.body, location);
}

void traverse_call(target_space* space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.call.function, location);
	move_if_necessary(space, &obj->data.call.arguments, location);
	move_if_necessary(space, &obj->data.call.continuation, location);
}

void traverse_continuation(target_space* space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.continuation.call, location);
}

void traverse_reader_entry(target_space* space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.reader_entry.proc, location);
}

void traverse_syntax_object(target_space* space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.syntax_object.syntax, location);
	move_if_necessary(space, &obj->data.syntax_object.origin, location);
}

void traverse_internal_error(target_space* space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.internal_error.trace, location);
	move_if_necessary(space, &obj->data.internal_error.message, location);
}

void traverse_string_port(target_space* space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.string_port.string, location);
}

void traverse_delay(target_space* space, object* obj, object_location location) {
	move_if_necessary(space, &obj->data.delay.value, location);
}

void traverse_nothing(target_space* space, object* obj, object_location location) {
}

typedef void (traversal_function)(target_space* space, object* obj, object_location location);

traversal_function* traversal[type_count];

void traverse_object(target_space* space, object* obj, object_location location) {
	(*traversal[obj->type])(space, obj, location);
}

void clear_garbage(memory_space* memory, object** root, object_location location, char move_root) {
	char* next_free = memory->next_free;
	target_space space;
	space.direction = memory->direction;
	space.target = &next_free;
	char* next_traversed = next_free;
	
	if (move_root) {
		move_if_necessary(&space, root, location);
	}
	else {
		traverse_object(&space, *root, location);
	}
	
	while (next_free != next_traversed) {
		object* obj = (object*)next_traversed;
		traverse_object(&space, obj, location);
		next_traversed += space.direction * object_size(obj);
	}
	memory->next_free = next_free;
}

long used_heap_data(memory_space* space) {
	int offset = space->next_free - space->memory;
	if (space->direction == 1) {
		return offset;
	}
	else {
		return space->size - offset;
	}
}

char heap_full(memory_space* space, long additional_data) {
	long total_data = used_heap_data(space) + additional_data;
	char space_exceeded = total_data > space->size;
	char half_space_exceeded = total_data + max_stack_data > space->half_size;
	return space->fill ? space_exceeded : half_space_exceeded;
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
	space->fill = 0;
	space->memory = malloc(space->size);
	space->next_free = space->memory;
	if (space->memory == 0) {
		printf("out of memory\n");
		exit(0);
	}
	printf("heap is at %p\n", (void*)space->memory);
}

#define print_gc 0

typedef struct {
	char* old_memory;
} gc_result;

gc_result perform_gc_traversal(char is_major, long additional_data, object** root) {
	gc_result res;
	res.old_memory = NULL;
	object_location location;
	if (is_major) {
		long used_data = used_heap_data(&main_memory_space);
		location = location_heap;
		if (main_memory_space.fill) {
			if (print_gc) printf("major resizing gc\n");
			res.old_memory = main_memory_space.memory;
			long quarter_size = main_memory_space.half_size;
			while (used_data + additional_data > 4*quarter_size) {
				quarter_size *= 2;
			}
			init_memory_space(&main_memory_space, quarter_size);
		}
		else {
			if (print_gc) printf("major gc\n");
			reset_memory_space(&main_memory_space);
		}
		if (print_gc) printf("used heap data pre-gc: %lu\n", used_data);
	}
	else {
		location = location_stack;
		object* ls = mutation_references;
		while (!is_empty_list(ls)) {
			// move all stack objects referenced by the heap
			clear_garbage(&main_memory_space, &(ls->data.list.first), location, 0);
			ls = list_rest(ls);
		}
	}
	clear_garbage(&main_memory_space, &symbol_list, location, 1);
	clear_garbage(&main_memory_space, root, location, 1);
	mutation_references = empty_list();
	object* ls = malloc_references;
	while (!is_empty_list(ls)) {
		free(memory_reference_value(list_first(ls)));
		ls = list_rest(ls);
	}
	malloc_references = empty_list();
	return res;
}

long living_stack_memory(object** root) {
	long size = max_stack_data;
	object* ls = malloc_references;
	while (!is_empty_list(ls)) {
		// move all stack objects referenced by the heap
		size += memory_reference_size(list_first(ls));
		ls = list_rest(ls);
	}
	return size;
}
	
void perform_gc(object** root) {
	long additional_data = living_stack_memory(root);
	char is_major = heap_full(&main_memory_space, additional_data);
	gc_result res = perform_gc_traversal(is_major, additional_data, root);
	free(res.old_memory);
	if (is_major) {
		main_memory_space.fill = resize_at_next_major_gc(&main_memory_space);
		if (print_gc) printf("used heap data post-gc: %ld\n", used_heap_data(&main_memory_space));
	}
}

void init_memory_handling() {
	mutation_references = empty_list();
	malloc_references = empty_list();
	
	object_type t;
	for (t = type_none; t < type_count; t++) {
		traversal[t] = &traverse_nothing;
	}
	traversal[type_symbol] = &traverse_symbol;
	traversal[type_integer] = &traverse_integer;
	traversal[type_fraction] = &traverse_fraction;
	traversal[type_complex] = &traverse_complex;
	traversal[type_list] = &traverse_list;
	traversal[type_stream] = &traverse_stream;
	traversal[type_vector] = &traverse_vector;
	traversal[type_vector_iterator] = &traverse_vector_iterator;
	traversal[type_struct_definition] = &traverse_struct_definition;
	traversal[type_struct_instance] = &traverse_struct_instance;
	traversal[type_function] = &traverse_function;
	traversal[type_binding] = &traverse_binding;
	traversal[type_call] = &traverse_call;
	traversal[type_continuation] = &traverse_continuation;
	traversal[type_reader_entry] = &traverse_reader_entry;
	traversal[type_syntax_object] = &traverse_syntax_object;
	traversal[type_internal_error] = &traverse_internal_error;
	traversal[type_string_port] = &traverse_string_port;
	traversal[type_delay] = &traverse_delay;
}

void init_memory_spaces(int size) {
	init_memory_space(&main_memory_space, size);
}
