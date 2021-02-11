#pragma once

#include "data-structures.h"

object** repl_scope_references(void);
void reset_repl_scope_references(void);
void add_repl_scope_reference(object* ls, object* obj);
#define alloc_repl_scope_reference(a) add_repl_scope_reference(alloc_obj(), a)

object* rewound_repl_scope_reference(object* key);
