#pragma once

#include "data-structures.h"

#define max_stack_data 1024*1024

void perform_gc(object** root);

void init_memory_handling();
void init_memory_spaces(int size);
