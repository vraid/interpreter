#include "repl-top.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "memory-handling.h"
#include "read.h"
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
	object* read_table;
	object* environment;
	delist_4(args, &value, &error_string, &read_table, &environment);
	
	if (is_internal_error(value)) {		
		printf("error:\n");
		
		object* print_args = alloc_list_3(value, read_table, environment);
		object* print_call = alloc_call(&repl_print_entry_proc, print_args, cont);
		
		return perform_call(print_call);
	}
	else {
		return call_cont(cont, value);
	}
}

object* repl_read_entry(object* args, object* cont) {
	object* read_table;	
	object* environment;
	delist_2(args, &read_table, &environment);
	
	object* validate_call = alloc_call(&repl_validate_entry_proc, args, cont);
	
	object* error_args = alloc_list_3(&read_error_string, read_table, environment);
	object* error_call = alloc_call(&repl_catch_error_proc, error_args, alloc_cont(validate_call));
	object* error_cont = alloc_catching_cont(error_call);
	
	object input_port;
	init_file_port(&input_port, stdin);
	
	object* read_args = alloc_list_2(&input_port, read_table);
	object* read_call = alloc_call(&read_entry_proc, read_args, error_cont);
	
	return perform_call(read_call);
}

object* repl_validate_entry(object* args, object* cont) {
	object* value;
	object* read_table;
	object* environment;
	delist_3(args, &value, &read_table, &environment);
	
	object* eval_args = list_rest(args);
	object* eval_call = alloc_call(&repl_eval_entry_proc, eval_args, cont);
	
	object* catch_args = alloc_list_3(&syntax_error_string, read_table, environment);
	object* catch_call = alloc_call(&repl_catch_error_proc, catch_args, alloc_cont(eval_call));
	object* catch_cont = alloc_catching_cont(catch_call);
	
	object* validate_args = alloc_list_3(value, environment, empty_list());
	object* validate_call = alloc_call(&validate_expression_proc, validate_args, catch_cont);
	
	return perform_call(validate_call);
}

object* repl_eval_entry(object* args, object* cont) {
	object* value;
	object* read_table;
	object* environment;
	delist_3(args, &value, &read_table, &environment);
	
	object* print_args = list_rest(args);
	object* print_call = alloc_call(&repl_print_or_read_proc, print_args, cont);
	
	object* error_args = alloc_list_3(&eval_error_string, read_table, environment);
	object* error_call = alloc_call(&repl_catch_error_proc, error_args, alloc_cont(print_call));
	object* error_cont = alloc_catching_cont(error_call);
	
	object* eval_args = alloc_list_3(value, environment, empty_list());
	object* eval_call = alloc_call(&eval_proc, eval_args, error_cont);
	
	return perform_call(eval_call);
}

object* repl_print_or_read(object* args, object* cont) {
	object* value;
	object* read_table;
	object* environment;
	delist_3(args, &value, &read_table, &environment);
	
	object* call;
	
	if (is_nonempty_list(value) && is_binding(list_first(value))) {
		object* read_args = alloc_list_2(read_table, value);
		call = alloc_call(&repl_read_entry_proc, read_args, cont);
	}
	else {
		call = alloc_call(&repl_print_entry_proc, args, cont);
	}
	
	return perform_call(call);
}

object* repl_print_entry(object* args, object* cont) {
	object* value;
	object* read_table;
	object* environment;
	delist_3(args, &value, &read_table, &environment);
	
	object* read_args = list_rest(args);
	object* read_call = alloc_call(&repl_read_entry_proc, read_args, cont);
	object* next_cont = alloc_discarding_cont(read_call);
	
	object* newline_call = alloc_call(&print_newline_proc, empty_list(), next_cont);
	object* newline_cont = alloc_discarding_cont(newline_call);
	
	object* print_args = alloc_list_1(value);
	object* call = alloc_call(&print_entry_proc, print_args, newline_cont);
	
	return perform_call(call);
}

object* repl_init(object* args, object* cont) {
	object* read_table;
	object* environment;
	delist_2(args, &read_table, &environment);
	
	object* call = alloc_call(&repl_read_entry_proc, alloc_list_2(read_table, environment), cont);
	alloc_repl_scope_reference(alloc_list_2(no_symbol(), call));
	
	return perform_call(call);
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
	init_primitive(&repl_init, &repl_init_proc);
	
	init_primitive(&repl_catch_error, &repl_catch_error_proc);
	
	init_static_string(&read_error_string, "read error");
	init_static_string(&syntax_error_string, "syntax error");
	init_static_string(&eval_error_string, "eval error");
}
