#pragma once

#include "data-structures.h"
#include "object-init.h"

object first_proc;
object rest_proc;
object take_proc;
object drop_proc;

void init_sequence_procedures(void);

int sequence_length(object* obj);
object* sequence_first(object* obj);
object* sequence_rest(object* iter, object* obj);
#define alloc_sequence_rest(a) sequence_rest(alloc_obj(), a)

object* list_to_sequence_proc(object_type type);

// returns first iterator for non-empty non-lists
// returns identity of a list
object* first_iterator(object* iter, object* obj);
#define alloc_first_iterator(a) first_iterator(alloc_obj(), a)

// inits and returns next for non-empty, non-lists
// returns rest of a list
object* next_iterator(object* next, object* current);
#define alloc_next_iterator(a) next_iterator(alloc_obj(), a)
