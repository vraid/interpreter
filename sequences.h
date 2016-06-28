#pragma once

#include "data-structures.h"

object take_proc;
object drop_proc;

void init_sequence_procedures(void);

int sequence_length(object* obj);
object* sequence_first(object* obj);
object* sequence_rest(object* iter, object* obj);

object* list_to_sequence_proc(object_type type);

object* first_proc(void);
object* rest_proc(void);

// returns first iterator for non-empty non-lists
// returns identity of a list
object* first_iterator(object* iter, object* obj);

// inits and returns next for non-empty, non-lists
// returns rest of a list
object* next_iterator(object* next, object* current);
