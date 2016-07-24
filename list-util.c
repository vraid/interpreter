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

object reverse_proc;

object* reverse(object* args, object* cont) {
	object* next;
	object* reversed;
	delist_2(args, &next, &reversed);
	
	if (is_empty_list(next)) {
		return call_cont(cont, reversed);
	}
	else {
		object cell;
		init_list_cell(&cell, list_first(next), reversed);
		
		object ls[2];
		init_list_2(ls, list_rest(next), &cell);
		object call;
		init_call(&call, &reverse_proc, ls, cont);
		
		return perform_call(&call);
	}
}

object* reverse_list(object* args, object* cont) {
	object* list;
	delist_1(args, &list);
	
	object ls[2];
	init_list_2(ls, list, empty_list());
	object call;
	init_call(&call, &reverse_proc, ls, cont);
	
	return perform_call(&call);
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

object* list_append_multiple(object* lists, int list_count, object* cont) {
	int length = 0;
	int i;
	object* ls = lists;
	for (i = 0; i < list_count-1; i++) {
		length += list_length(list_first(ls));
		ls = list_rest(ls);
	}
	object* last = list_first(ls);
	
	if (length == 0) {
		return call_cont(cont, last);
	}
	else {
		// one extra length to avoid corner case on last cell
		object* new_list = alloca(sizeof(object) * (length + 1));
		
		i = 0;
		int k;
		for (k = 0; k < list_count-1; k++) {
			ls = list_first(lists);
			while (!is_empty_list(ls)) {
				init_list_cell(&new_list[i], list_first(ls), &new_list[i+1]);
				ls = list_rest(ls);
				i++;
			}
			lists = list_rest(lists);
		}
		new_list[length-1].data.list.rest = last;
		
		return call_cont(cont, new_list);
	}
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
		return list_append_multiple(lists, list_count, cont);
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
		int length = list_length(first);
		object* ls = alloca(sizeof(object) * length);
		
		ls[0].data.list.rest = second;
		int i;
		for (i = 0; i < length; i++) {
			if (i > 0) ls[i].data.list.rest = &ls[i-1];
			ls[i].data.list.first = list_first(first);
			first = list_rest(first);
		}
		
		return call_cont(cont, &ls[length-1]);
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
	init_primitive_procedure(&make_list_proc, &make_list);
	init_primitive_procedure(&add_to_list_proc, &add_to_list);
	init_primitive_procedure(&return_list_proc, &return_list);
	init_primitive_procedure(&reverse_list_proc, &reverse_list);
	init_primitive_procedure(&reverse_proc, &reverse);
	init_primitive_procedure(&list_append_proc, &list_append);
	init_primitive_procedure(&list_append_first_reversed_proc, &list_append_first_reversed);
	init_primitive_procedure(&unzip_2_proc, &unzip_2);
	init_primitive_procedure(&unzip_2_step_proc, &unzip_2_step);
}
