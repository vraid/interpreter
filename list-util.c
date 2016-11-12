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

object return_list_proc;

object* return_list(object* args, object* cont) {
	object* first;
	delist_1(args, &first);
	
	return call_cont(cont, first);
}

object* make_list(object* args, object* cont) {
	object* value;
	object* proc;
	object* proc_args;
	delist_3(args, &value, &proc, &proc_args);
	
	object first;
	init_list_1(&first, value);
	
	object return_args[1];
	init_list_1(return_args, &first);
	object return_call;
	init_call(&return_call, &return_list_proc, return_args, cont);
	object return_cont;
	init_discarding_cont(&return_cont, &return_call);
	
	object call_args;
	init_list_cell(&call_args, &first, proc_args);
	object call;
	init_call(&call, proc, &call_args, &return_cont);
	
	return perform_call(&call);
}

object* add_to_list(object* args, object* cont) {
	object* value;
	object* last;
	delist_2(args, &value, &last);
	
	object cell;
	init_list_1(&cell, value);
	last->data.list.rest = &cell;
	add_mutation(last, &cell);
	
	return call_cont(cont, &cell);
}

object* reverse_list(object* args, object* cont) {
	object* list;
	delist_1(args, &list);
	
	object* next = empty_list();
	
	while (!is_empty_list(list)) {
		object* cell = alloca(sizeof(object));
		init_list_cell(cell, list_first(list), next);
		list = list_rest(list);
		next = cell;
	}
	
	return call_cont(cont, next);
}

object unzip_2_step_proc;

object* unzip_2_step(object* args, object* cont) {
	object* one;
	object* two;
	object* list;
	delist_3(args, &one, &two, &list);
	
	if (is_empty_list(list)) {
		return call_discarding_cont(cont);
	}
	else {
		object* first = list_first(list);
		object* a;
		object* b;
		delist_2(first, &a, &b);
		object one_next[1];
		init_list_1(one_next, a);
		object two_next[1];
		init_list_1(two_next, b);
		one->data.list.rest = one_next;
		add_mutation(one, one_next);
		two->data.list.rest = two_next;
		add_mutation(two, two_next);
		
		object call_args[3];
		init_list_3(call_args, one_next, two_next, list_rest(list));
		object call;
		init_call(&call, &unzip_2_step_proc, call_args, cont);
		
		return perform_call(&call);
	}
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
			object* cell = alloca(sizeof(object));
			init_list_cell(cell, list_first(ls), empty_list());
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
			object* cell = alloca(sizeof(object));
			init_list_cell(cell, list_first(first), next);
			next = cell;
			first = list_rest(first);
		}
		
		return call_cont(cont, next);
	}
}

object* unzip_2(object* args, object* cont) {
	object* list;
	delist_1(args, &list);
	
	if (is_empty_list(list)) {
		return call_cont(cont, empty_list());
	}
	else {
		object* first = list_first(list);
		object* a;
		object* b;
		delist_2(first, &a, &b);
		object one[1];
		init_list_1(one, a);
		object two[1];
		init_list_1(two, b);
		
		object result[2];
		init_list_2(result, one, two);
		
		object result_args[1];
		init_list_1(result_args, result);
		object result_call;
		init_call(&result_call, &identity_proc, result_args, cont);
		object result_cont;
		init_discarding_cont(&result_cont, &result_call);
		
		object unzip_args[3];
		init_list_3(unzip_args, one, two, list_rest(list));
		object unzip_call;
		init_call(&unzip_call, &unzip_2_step_proc, unzip_args, &result_cont);
		
		return perform_call(&unzip_call);
	}
}

void init_list_util_procedures(void) {
	init_primitive(&make_list, &make_list_proc);
	init_primitive(&add_to_list, &add_to_list_proc);
	init_primitive(&return_list, &return_list_proc);
	init_primitive(&reverse_list, &reverse_list_proc);
	init_primitive(&list_append, &list_append_proc);
	init_primitive(&list_append_first_reversed, &list_append_first_reversed_proc);
	init_primitive(&unzip_2, &unzip_2_proc);
	init_primitive(&unzip_2_step, &unzip_2_step_proc);
}
