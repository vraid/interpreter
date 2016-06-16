#include "list-util.h"
#include "global-variables.h"
#include "object-init.h"
#include "call.h"
#include "delist.h"
#include "memory-handling.h"

object _add_to_list_proc;
object _finish_list_proc;
object _reverse_list_proc;

object* add_to_list_proc(void) {
	return &_add_to_list_proc;
}

object* finish_list_proc(void) {
	return &_finish_list_proc;
}

object* reverse_list_proc(void) {
	return &_reverse_list_proc;
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

object* finish_list(object* args, object* cont) {
	object* first;
	delist_1(args, &first);
	
	return call_cont(cont, first);
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
	init_primitive_procedure(add_to_list_proc(), &add_to_list);
	init_primitive_procedure(finish_list_proc(), &finish_list);
	init_primitive_procedure(reverse_list_proc(), &reverse_list);
	init_primitive_procedure(&reverse_proc, &reverse);
}
