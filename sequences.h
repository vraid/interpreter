#pragma once

#include "data-structures.h"
#include "object-init.h"

object first_proc;
object rest_proc;
object take_proc;
object drop_proc;

typedef object* (sequence_first_func)(object* seq);
object* sequence_first(object* seq);
void add_first_procedure(object_type type, sequence_first_func* proc);
void add_rest_procedure(object_type type, object* proc);

void init_sequence_procedures(void);
