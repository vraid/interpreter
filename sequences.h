#pragma once

#include "data-structures.h"

int sequence_length(object* obj);
object* sequence_first(object* obj);

// returns first iterator for non-empty non-lists
// returns identity of a list
// returns the empty list for empty sequences
object* first_iterator(object* iter, object* obj);

// inits and returns next for non-empty, non-lists
// returns rest of a list
// returns the empty list for the end of a sequence
object* next_iterator(object* next, object* current);
