#include "repl-top.h"
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "read.h"
#include "eval.h"
#include "print.h"
#include "delist.h"
#include "call.h"

object repl_eval_entry_proc;
object repl_print_or_read_proc;
object repl_print_entry_proc;

object read_error_string;
object eval_error_string;
object repl_error_proc;

object* repl_error(object* args, object* cont) {
	object* value;
	object* error_string;
	object* environment;
	delist_3(args, &value, &error_string, &environment);
	
	if (is_internal_error(value)) {
		printf("%s: %s\n", string_value(error_string), string_value(internal_error_message(value)));
		
		object read_args[1];
		init_list_1(read_args, environment);
		object read_call;
		init_call(&read_call, &repl_read_entry_proc, read_args, end_cont());
		
		return perform_call(&read_call);
	}
	else {
		return call_cont(cont, value);
	}
}

object* repl_read_entry(object* args, object* cont) {
	object* environment;
	delist_1(args, &environment);
	
	object eval_call;
	init_call(&eval_call, &repl_eval_entry_proc, args, cont);
	object eval_cont;
	init_cont(&eval_cont, &eval_call);
	
	object error_args[2];
	init_list_2(error_args, &read_error_string, environment);
	object error_call;
	init_call(&error_call, &repl_error_proc, error_args, &eval_cont);
	object error_cont;
	init_catching_cont(&error_cont, &error_call);
	
	object input_port;
	init_object(location_stack, type_file_port, &input_port);
	input_port.data.file_port.file = stdin;
	
	object read_args[1];
	init_list_1(read_args, &input_port);
	object read_call;
	init_call(&read_call, &read_value_proc, read_args, &error_cont);
	
	return perform_call(&read_call);
}

object* repl_eval_entry(object* args, object* cont) {
	object* value;
	object* environment;
	delist_2(args, &value, &environment);
	
	object print_args[1];
	init_list_1(print_args, environment);
	object print_call;
	init_call(&print_call, &repl_print_or_read_proc, print_args, cont);
	object print_cont;
	init_cont(&print_cont, &print_call);
	
	object error_args[2];
	init_list_2(error_args, &eval_error_string, environment);
	object error_call;
	init_call(&error_call, &repl_error_proc, error_args, &print_cont);
	object error_cont;
	init_catching_cont(&error_cont, &error_call);
	
	object eval_call;
	init_call(&eval_call, &eval_proc, args, &error_cont);
	
	return perform_call(&eval_call);
}

object* repl_print_or_read(object* args, object* cont) {
	object* value;
	object* environment;
	delist_2(args, &value, &environment);
	
	object call;
	
	if (is_internal_error(value)) {
		printf("error: %s\n", string_value(internal_error_message(value)));
		init_call(&call, &repl_read_entry_proc, args, cont);
	}
	if (is_environment(value)) {
		object ls[1];
		init_list_1(ls, value);
		init_call(&call, &repl_read_entry_proc, ls, cont);
	}
	else {
		init_call(&call, &repl_print_entry_proc, args, cont);
	}
	
	return perform_call(&call);
}

object* repl_print_entry(object* args, object* cont) {
	object* value;
	object* environment;
	delist_2(args, &value, &environment);
	
	object ls[1];
	init_list_1(ls, environment);
	
	object read_call;
	init_call(&read_call, &repl_read_entry_proc, ls, cont);
	object next_cont;
	init_discarding_cont(&next_cont, &read_call);
	
	object newline_call;
	init_call(&newline_call, &print_newline_proc, empty_list(), &next_cont);
	object newline_cont;
	init_discarding_cont(&newline_cont, &newline_call);
	
	object ls2[1];
	init_list_1(ls2, value);
	object call;
	init_call(&call, &print_proc, ls2, &newline_cont);
	
	return perform_call(&call);
}

void init_repl_procedures(void) {
	init_primitive(&repl_read_entry, &repl_read_entry_proc);
	init_primitive(&repl_eval_entry, &repl_eval_entry_proc);
	init_primitive(&repl_print_or_read, &repl_print_or_read_proc);
	init_primitive(&repl_print_entry, &repl_print_entry_proc);
	
	init_primitive(&repl_error, &repl_error_proc);
	
	init_string(&read_error_string, "read error");
	make_static(&read_error_string);
	init_string(&eval_error_string, "eval error");
	make_static(&eval_error_string);
}
