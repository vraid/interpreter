#include "list-util.h"
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "call.h"
#include "delist.h"
#include "memory-handling.h"

object _first_proc;
object _rest_proc;

object _make_list_proc;
object _add_to_list_proc;
object _reverse_list_proc;

object* first_proc(void) {
	return &_first_proc;
}

object* rest_proc(void) {
	return &_rest_proc;
}

object* make_list_proc(void) {
	return &_make_list_proc;
}

object* add_to_list_proc(void) {
	return &_add_to_list_proc;
}

object* reverse_list_proc(void) {
	return &_reverse_list_proc;
}

object* first(object* args, object* cont) {
	object* list;
	delist_1(args, &list);
	
	list = unquote(list);
	
	if (!is_list(list)) {
		return throw_error(cont, "first on non-list");
	}
	else {
		return call_cont(cont, list_first(list));
	}
}

object* rest(object* args, object* cont) {
	object* list;
	delist_1(args, &list);
	
	list = unquote(list);
	
	if (!is_list(list)) {
		return throw_error(cont, "rest on non-list");
	}
	else {
		return call_cont(cont, list_rest(list));
	}
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

void init_list_util_procedures(void) {
	init_primitive_procedure(first_proc(), &first);
	init_primitive_procedure(rest_proc(), &rest);
	init_primitive_procedure(make_list_proc(), &make_list);
	init_primitive_procedure(add_to_list_proc(), &add_to_list);
	init_primitive_procedure(&return_list_proc, &return_list);
	init_primitive_procedure(reverse_list_proc(), &reverse_list);
	init_primitive_procedure(&reverse_proc, &reverse);
}
