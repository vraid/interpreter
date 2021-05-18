#pragma once

#include "data-structures.h"

object vector_to_list_proc;
object list_to_vector_proc;

void init_vector_procedures(void);

object* vector_ref(object* vec, int n);
int vector_find(object* vec, object* a);
