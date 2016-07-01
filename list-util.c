#include "list-util.h"
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
	init_primitive_procedure(&unzip_2_proc, &unzip_2);
	init_primitive_procedure(&unzip_2_step_proc, &unzip_2_step);
}
