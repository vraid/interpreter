#pragma once

#include "data-structures.h"

typedef enum {
	forwards,
	backwards} direction;

typedef struct {
	int size;
	direction memory_direction;
	object* memory;
} memory_space;

void clear_garbage(object* to_space, object* root, int direction, object_location location);