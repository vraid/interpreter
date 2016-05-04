#include "repl-top.h"
#include "allocation.h"
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "eval.h"
#include "print.h"
#include "delist.h"
#include "call.h"

object repl_read_entry_proc;
object repl_eval_entry_proc;
object repl_print_or_read_proc;
object repl_print_entry_proc;

object placeholder_read_proc;
object placeholder_eval_proc;
object placeholder_print_proc;

object* placeholder_read(object* args, object* cont) {
	object* input_port;
	delist_1(args, &input_port);

	printf("read\n");
	getc(file_port_file(input_port));
	
	return call_cont(cont, no_object());
}

object* placeholder_eval(object* args, object* cont) {
	printf("eval\n");
	
	return call_cont(cont, no_object());
}

object* placeholder_print(object* args, object* cont) {
	printf("print\n");
	
	return call_cont(cont, no_object());
}

object* repl_read_entry(object* args, object* cont) {
	object* value;
	object* environment;
	delist_2(args, &value, &environment);
	
	object ls[1];
	init_list_1(ls, environment);
	
	object eval_call;
	init_call(&eval_call, &repl_eval_entry_proc, ls, cont);
	object next_cont;
	init_cont(&next_cont, &eval_call);
	
	object input_port;
	input_port.type = type_file_port;
	input_port.data.file_port.file = stdin;
	
	object ls2[1];
	init_list_1(ls2, &input_port);
	
	object call;
	init_call(&call, &placeholder_read_proc, ls2, &next_cont);
	
	return perform_call(&call);
}

object* repl_eval_entry(object* args, object* cont) {
	object* value;
	object* environment;
	delist_2(args, &value, &environment);
	
	object ls[1];
	init_list_1(ls, environment);
	
	object print_call;
	init_call(&print_call, &repl_print_or_read_proc, ls, cont);
	object next_cont;
	init_cont(&next_cont, &print_call);
	
	object call;
	init_call(&call, &placeholder_eval_proc, args, &next_cont);
	
	return perform_call(&call);
}

object* repl_print_or_read(object* args, object* cont) {
	object* value;
	object* environment;
	delist_2(args, &value, &environment);
	
	object call;
	
	if (is_environment(value)) {
		init_call(&call, &repl_read_entry_proc, args, cont);
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
	init_cont(&next_cont, &read_call);
	
	object call;
	init_call(&call, &placeholder_print_proc, args, &next_cont);
	
	return perform_call(&call);
}

void init_repl_procedures(void) {
	init_primitive_procedure(&repl_read_entry_proc, &repl_read_entry);
	init_primitive_procedure(&repl_eval_entry_proc, &repl_eval_entry);
	init_primitive_procedure(&repl_print_or_read_proc, &repl_print_or_read);
	init_primitive_procedure(&repl_print_entry_proc, &repl_print_entry);
	
	init_primitive_procedure(&placeholder_read_proc, &placeholder_read);
	init_primitive_procedure(&placeholder_eval_proc, &placeholder_eval);
	init_primitive_procedure(&placeholder_print_proc, &placeholder_print);
}
