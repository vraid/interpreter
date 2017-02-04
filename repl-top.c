#include "repl-top.h"
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "read.h"
#include "read-state.h"
#include "eval.h"
#include "print.h"
#include "syntax-validate.h"
#include "delist.h"
#include "call.h"

object repl_validate_entry_proc;
object repl_eval_entry_proc;
object repl_print_or_read_proc;
object repl_print_entry_proc;

object read_error_string;
object syntax_error_string;
object eval_error_string;
object repl_catch_error_proc;

object* repl_catch_error(object* args, object* cont) {
	object* value;
	object* error_string;
	object* environment;
	delist_3(args, &value, &error_string, &environment);
	
	if (is_internal_error(value)) {		
		printf("%s:\n", string_value(error_string));
		
		object print_args[2];
		init_list_2(print_args, internal_error_message(value), environment);
		object print_call;
		init_call(&print_call, &repl_print_entry_proc, print_args, cont);
		
		return perform_call(&print_call);
	}
	else {
		return call_cont(cont, value);
	}
}

object* repl_read_entry(object* args, object* cont) {
	object* environment;
	delist_1(args, &environment);
	
	object validate_call;
	init_call(&validate_call, &repl_validate_entry_proc, args, cont);
	object validate_cont;
	init_cont(&validate_cont, &validate_call);
	
	object error_args[2];
	init_list_2(error_args, &read_error_string, environment);
	object error_call;
	init_call(&error_call, &repl_catch_error_proc, error_args, &validate_cont);
	object error_cont;
	init_catching_cont(&error_cont, &error_call);
	
	object input_port;
	init_object(location_stack, type_file_port, &input_port);
	input_port.data.file_port.file = stdin;
	
	object read_args[1];
	init_list_1(read_args, &input_port);
	object read_call;
	init_call(&read_call, &read_entry_proc, read_args, &error_cont);
	
	return perform_call(&read_call);
}

object* repl_validate_entry(object* args, object* cont) {
	object* value;
	object* environment;
	delist_2(args, &value, &environment);
	
	object eval_args[1];
	init_list_1(eval_args, environment);
	object eval_call;
	init_call(&eval_call, &repl_eval_entry_proc, eval_args, cont);
	object eval_cont;
	init_cont(&eval_cont, &eval_call);
	
	object catch_args[2];
	init_list_2(catch_args, &syntax_error_string, environment);
	object catch_call;
	init_call(&catch_call, &repl_catch_error_proc, catch_args, &eval_cont);
	object catch_cont;
	init_catching_cont(&catch_cont, &catch_call);
	
	object validate_call;
	init_call(&validate_call, &validate_expression_proc, args, &catch_cont);
	object validate_cont;
	init_cont(&validate_cont, &validate_call);
	
	return perform_call(&validate_call);
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
	init_call(&error_call, &repl_catch_error_proc, error_args, &print_cont);
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
	
	object read_args[1];
	init_list_1(read_args, environment);
	
	object read_call;
	init_call(&read_call, &repl_read_entry_proc, read_args, cont);
	object next_cont;
	init_discarding_cont(&next_cont, &read_call);
	
	object newline_call;
	init_call(&newline_call, &print_newline_proc, empty_list(), &next_cont);
	object newline_cont;
	init_discarding_cont(&newline_cont, &newline_call);
	
	object print_args[1];
	init_list_1(print_args, value);
	object call;
	init_call(&call, &print_entry_proc, print_args, &newline_cont);
	
	return perform_call(&call);
}

void init_static_string(object* obj, char* str) {
	init_string(obj, str);
	make_static(obj);
}

void init_repl_procedures(void) {
	init_primitive(&repl_read_entry, &repl_read_entry_proc);
	init_primitive(&repl_validate_entry, &repl_validate_entry_proc);
	init_primitive(&repl_eval_entry, &repl_eval_entry_proc);
	init_primitive(&repl_print_or_read, &repl_print_or_read_proc);
	init_primitive(&repl_print_entry, &repl_print_entry_proc);
	
	init_primitive(&repl_catch_error, &repl_catch_error_proc);
	
	init_static_string(&read_error_string, "read error");
	init_static_string(&syntax_error_string, "syntax error");
	init_static_string(&eval_error_string, "eval error");
}
