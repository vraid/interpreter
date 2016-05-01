#include "read.h"
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "delist.h"
#include "call.h"

object read_value_proc;
object add_to_list_proc;
object read_list_value_proc;
object read_list_proc;

object* read_value(object* args, object* cont) {
	object* input_port;
	delist_1(args, &input_port);
	object value;
	
	return call_cont(cont, &value);
}

object* add_to_list(object* args, object* cont) {
	object* value;
	object* last;
	delist_2(args, &value, &last);
	
	object next;
	
	last->data.list.rest = &next;
	init_list_cell(&next, value, empty_list());
	
	return call_cont(cont, &next);
}

object* read_list_value(object* args, object* cont) {
	object next_call;
	init_call(&next_call, &add_to_list_proc, args, cont);
	object next_cont;
	init_cont(&next_cont, &next_call);
	object call;
	init_call(&call, &read_value_proc, args, &next_cont);
	
	return perform_call(&call);
}

object* read_list(object* args, object* cont) {
	object* input;
	object* first;
	object* last;
	delist_3(args, &first, &last, &input);
	// if end list
	// call_cont(cont, first)
	// else
	object call;
	init_call(&call, &read_list_value_proc, args, cont);
	return perform_call(&call);
}

void init_read_procedures(void) {
	init_primitive_procedure(&read_value_proc, &read_value);
	init_primitive_procedure(&add_to_list_proc, &add_to_list);
	init_primitive_procedure(&read_list_value_proc, &read_list_value);
	init_primitive_procedure(&read_list_proc, &read_list);
}
