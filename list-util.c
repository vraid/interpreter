#include "list-util.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "call.h"
#include "delist.h"
#include "base-util.h"
#include "memory-handling.h"

int list_length(object* ls) {
	int n = 0;
	while (!is_empty_list(ls)) {
		n++;
		ls = list_rest(ls);
	}
	return n;
}

object* list_ref(int n, object* ls) {
	while (n > 0) {
		ls = list_rest(ls);
		n--;
	}
	return list_first(ls);
}

object* find_duplicate(object* a) {
	while (!is_empty_list(a)) {
		object* obj = list_first(a);
		object* b = list_rest(a);
		while (!is_empty_list(b)) {
			if (obj == list_first(b)) {
				return obj;
			}
			b = list_rest(b);
		}
		a = list_rest(a);
	}
	return false();
}

object* find_duplicate_2(object* a, object* b) {
	while (!is_empty_list(a)) {
		object* obj = list_first(a);
		object* ls = b;
		while (!is_empty_list(ls)) {
			if (obj == list_first(ls)) {
				return obj;
			}
			ls = list_rest(ls);
		}
		a = list_rest(a);
	}
	return false();
}

object* make_list(object* args, object* cont) {
	object* value;
	object* proc;
	object* proc_args;
	delist_3(args, &value, &proc, &proc_args);
	
	object* first = alloc_list_1(value);
	
	object* return_args = alloc_list_1(first);
	object* return_call = alloc_call(&identity_proc, return_args, cont);
	object* return_cont = alloc_discarding_cont(return_call);
	
	object* call_args = alloc_list_cell(first, proc_args);
	object* call = alloc_call(proc, call_args, return_cont);
	
	return perform_call(call);
}

object* link_list(object* args, object* cont) {
	object* value;
	object* list;
	delist_2(args, &value, &list);
	
	object* cell = alloc_list_cell(value, list);
	
	return call_cont(cont, cell);
}

object* add_to_list(object* args, object* cont) {
	object* value;
	object* last;
	delist_2(args, &value, &last);
	
	object* cell = alloc_list_1(value);
	last->data.list.rest = cell;
	alloc_stack_reference(last, cell);
	
	return call_cont(cont, cell);
}

object* reverse_list(object* args, object* cont) {
	object* list;
	delist_1(args, &list);
	
	object* next = empty_list();
	
	while (!is_empty_list(list)) {
		object* cell = alloc_list_cell(list_first(list), next);
		list = list_rest(list);
		next = cell;
	}
	
	return call_cont(cont, next);
}

object unzip_2_next_proc;

object* unzip_2_next(object* args, object* cont) {
	object* list;
	delist_1(args, &list);
	
	object* ls[2];
	delist_2(list, &ls[0], &ls[1]);
	
	object* next[2];
	next[0] = empty_list();
	next[1] = empty_list();
	
	while (!is_empty_list(ls[0])) {
		int i;
		for (i = 0; i < 2; i++) {
			next[i] = alloc_list_cell(list_first(ls[i]), next[i]);
			ls[i] = list_rest(ls[i]);
		}
	}
	
	object* res = alloc_list_2(next[0], next[1]);
	return call_cont(cont, res);
}

object* unzip_2(object* args, object* cont) {	
	object* reverse_call = alloc_call(&unzip_2_next_proc, empty_list(), cont);
	object* reverse_cont = alloc_cont(reverse_call);
	
	object* unzip_call = alloc_call(&unzip_2_reversed_proc, args, reverse_cont);
	
	return perform_call(unzip_call);
}

object* unzip_2_reversed(object* args, object* cont) {
	object* list;
	delist_1(args, &list);
	
	object* next[2];
	next[0] = empty_list();
	next[1] = empty_list();
	
	while (!is_empty_list(list)) {
		object* first = desyntax(list_first(list));
		object* a[2];
		delist_2(first, &a[0], &a[1]);
		int i;
		for (i = 0; i < 2; i++) {
			next[i] = alloc_list_cell(a[i], next[i]);
		}
		list = list_rest(list);
	}
	
	object* res = alloc_list_2(next[0], next[1]);
	return call_cont(cont, res);
}

// full stack allocation, may fail on large lists
// should be changed to allocate in steps for large lists

object* list_append_multiple(object* lists, object* cont) {
	object* last = empty_list();
	object* first = last;
	char is_first = 1;
	
	while (!is_empty_list(lists)) {
		object* ls = list_first(lists);
		while (!is_empty_list(ls)) {
			object* cell = alloc_list_cell(list_first(ls), empty_list());
			if (is_first) {
				first = cell;
				is_first = 0;
			}
			if (!is_empty_list(last)) {
				last->data.list.rest = cell;
			}
			ls = list_rest(ls);
			last = cell;
		}
		lists = list_rest(lists);
	}
	
	return call_cont(cont, first);
}

object* list_append(object* args, object* cont) {
	object* lists;
	delist_1(args, &lists);
	
	int list_count = 0;
	object* ls = lists;
	while (!is_empty_list(ls)) {
		ls = list_rest(ls);
		list_count++;
	}
	
	if (list_count == 0) {
		return call_cont(cont, empty_list());
	}
	else if (list_count == 1) {
		return call_cont(cont, list_first(lists));
	}
	else {
		return list_append_multiple(lists, cont);
	}
}

object* list_append_first_reversed(object* args, object* cont) {
	object* first;
	object* second;
	delist_2(args, &first, &second);
	
	if (is_empty_list(first)) {
		return call_cont(cont, second);
	}
	else {
		object* next = second;
		
		while (!is_empty_list(first)) {
			object* cell = alloc_list_cell(list_first(first), next);
			next = cell;
			first = list_rest(first);
		}
		
		return call_cont(cont, next);
	}
}

void init_list_util_procedures(void) {
	init_primitive(&make_list, &make_list_proc);
	init_primitive(&link_list, &link_list_proc);
	init_primitive(&add_to_list, &add_to_list_proc);
	init_primitive(&reverse_list, &reverse_list_proc);
	init_primitive(&list_append, &list_append_proc);
	init_primitive(&list_append_first_reversed, &list_append_first_reversed_proc);
	init_primitive(&unzip_2, &unzip_2_proc);
	init_primitive(&unzip_2_next, &unzip_2_next_proc);
	init_primitive(&unzip_2_reversed, &unzip_2_reversed_proc);
}
