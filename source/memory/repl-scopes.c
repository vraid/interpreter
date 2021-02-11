#include "repl-scopes.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "delist.h"

object* _repl_scope_references;

object** repl_scope_references(void) {
	return &_repl_scope_references;
}

void reset_repl_scope_references(void) {
	_repl_scope_references = empty_list();
}

void add_repl_scope_reference(object* ls, object* obj) {
	_repl_scope_references = init_list_cell(ls, obj, _repl_scope_references);
}

object* rewound_repl_scope_reference(object* key) {
	object* ls = list_rest(_repl_scope_references);
	object* k;
	object* call;
	while (!is_empty_list(ls)) {
		delist_2(list_first(ls), &k, &call);
		if (is_no_symbol(key) || (key == k)) {
			_repl_scope_references = ls;
			return call;
		}
		ls = list_rest(ls);
	}
	return no_object();
}
