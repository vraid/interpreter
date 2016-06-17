#include "eval.h"

#include <stdlib.h>
#include <string.h>
#include "data-structures.h"
#include "object-init.h"
#include "global-variables.h"
#include "environments.h"
#include "call.h"
#include "delist.h"
#include "list-util.h"

object eval_identity_proc;
object eval_symbol_proc;
object eval_list_proc;
object eval_list_rest_proc;
object eval_function_proc;
object _eval_list_elements_proc;
object _eval_with_environment_proc;

object _eval_proc;

object* eval_proc(void) {
	return &_eval_proc;
}

object* eval_list_elements_proc(void) {
	return &_eval_list_elements_proc;
}

object* eval_with_environment_proc(void) {
	return &_eval_with_environment_proc;
}

char invalid_value_string[] = "invalid value: ";

object* eval_invalid_value(object* string, object* cont) {
	char* str = alloca(sizeof(char) * (1 + strlen(invalid_value_string) + string_length(string)));
	strcpy(str, invalid_value_string);
	strcpy(str + strlen(invalid_value_string), string_value(string));
	object message;
	init_string(&message, str);
	object e;
	init_internal_error(&e, &message);
	return call_cont(cont, &e);
}

object* eval_identity(object* args, object* cont) {
	object* obj;
	object* environment;
	delist_2(args, &obj, &environment);
	
	return call_cont(cont, obj);
}

object eval_syntax_proc;

object* eval_syntax(object* args, object* cont) {
	object* syntax;
	object* rest;
	object* environment;
	delist_3(args, &syntax, &rest, &environment);
	
	object ls[2];
	init_list_2(ls, rest, environment);
	
	object call;
	init_call(&call, syntax, ls, cont);
	
	return perform_call(&call);
}

object eval_primitive_procedure_call_proc;

object* eval_primitive_procedure_call(object* args, object* cont) {
	object* arguments;
	object* proc;
	object* environment;
	delist_3(args, &arguments, &proc, &environment);
	
	object call;
	init_call(&call, proc, arguments, cont);
	
	return perform_call(&call);
}

object eval_primitive_procedure_proc;

object* eval_primitive_procedure(object* args, object* cont) {
	object* proc;
	object* arguments;
	object* environment;
	delist_3(args, &proc, &arguments, &environment);
	
	object proc_ls[2];
	init_list_2(proc_ls, proc, environment);
	object proc_call;
	init_call(&proc_call, &eval_primitive_procedure_call_proc, proc_ls, cont);
	object proc_cont;
	init_cont(&proc_cont, &proc_call);
	
	object ls[2];
	init_list_2(ls, arguments, environment);
	object call;
	init_call(&call, eval_list_elements_proc(), ls, &proc_cont);
	
	return perform_call(&call);
}

object eval_function_body_proc;

object* eval_function_body(object* args, object* cont) {
	object* environment;
	object* body;
	delist_2(args, &environment, &body);

	object ls[2];
	init_list_2(ls, body, environment);

	object call;
	init_call(&call, eval_proc(), ls, cont);
	
	return perform_call(&call);
}

object* eval_function(object* args, object* cont) {
	object* function;
	object* arguments;
	object* environment;
	delist_3(args, &function, &arguments, &environment);
	
	environment = function_environment(function);
	
	object body_ls[1];
	init_list_1(body_ls, function_body(function));
	object eval_body_call;
	init_call(&eval_body_call, &eval_function_body_proc, body_ls, cont);
	object body_cont;
	init_cont(&body_cont, &eval_body_call);
	
	object bind_ls[2];
	init_list_2(bind_ls, function_parameters(function), environment);
	object bind_call;
	init_call(&bind_call, bind_values_proc(), bind_ls, &body_cont);
	object bind_cont;
	init_cont(&bind_cont, &bind_call);
	
	object args_ls[2];
	init_list_2(args_ls, arguments, environment);
	object eval_args_call;
	init_call(&eval_args_call, eval_list_elements_proc(), args_ls, &bind_cont);
	
	return perform_call(&eval_args_call);
}

object eval_list_elements_rest_proc;

object* eval_list_elements_rest(object* args, object* cont) {
	object* last;
	object* unevaled;
	object* environment;
	delist_3(args, &last, &unevaled, &environment);
	
	if (is_empty_list(unevaled)) {
		return call_discarding_cont(cont);
	}
	else {
		object ls[2];
		init_list_2(ls, list_rest(unevaled), environment);
		object build_call;
		init_call(&build_call, &eval_list_elements_rest_proc, ls, cont);
		object build_cont;
		init_cont(&build_cont, &build_call);

		object addls[1];
		init_list_1(addls, last);
		object add_call;
		init_call(&add_call, add_to_list_proc(), addls, &build_cont);
		object add_cont;
		init_cont(&add_cont, &add_call);
		
		object ls2[2];
		init_list_2(ls2, list_first(unevaled), environment);
		object eval_call;
		init_call(&eval_call, eval_proc(), ls2, &add_cont);
		return perform_call(&eval_call);
	}
}

object eval_list_elements_first_proc;

object* eval_list_elements_first(object* args, object* cont) {
	object* value;
	object* unevaled;
	object* environment;
	delist_3(args, &value, &unevaled, &environment);
	
	object first;
	init_list_cell(&first, value, empty_list());
	
	object ls[1];
	init_list_1(ls, &first);
	
	// after the list is finished, pass it on to quote
	object finish_call;
	init_call(&finish_call, finish_list_proc(), ls, cont);
	
	object finish_cont;
	init_discarding_cont(&finish_cont, &finish_call);
	
	object ls2[3];
	init_list_3(ls2, &first, unevaled, environment);
	
	// keep on building the list
	object call;
	init_call(&call, &eval_list_elements_rest_proc, ls2, &finish_cont);
	
	return perform_call(&call);
}

object* eval_list_elements(object* args, object* cont) {
	object* elements;
	object* environment;
	delist_2(args, &elements, &environment);
	
	if (is_empty_list(elements)) {
		return call_cont(cont, empty_list());
	}
	else {
		object ls[2];
		init_list_2(ls, list_rest(elements), environment);
		object build_call;
		init_call(&build_call, &eval_list_elements_first_proc, ls, cont);
		object build_cont;
		init_cont(&build_cont, &build_call);
		
		object ls2[2];
		init_list_2(ls2, list_first(elements), environment);
		object eval_call;
		init_call(&eval_call, eval_proc(), ls2, &build_cont);
		return perform_call(&eval_call);
	}
}

object* eval_list_rest(object* args, object* cont) {
	object* first;
	object* rest;
	object* environment;
	delist_3(args, &first, &rest, &environment);
	
	object* proc;
	
	switch (first->type) {
		case type_syntax:
			proc = &eval_syntax_proc;
			break;
		case type_primitive_procedure:
			proc = &eval_primitive_procedure_proc;
			break;
		case type_function:
			proc = &eval_function_proc;
			break;
		default:
			fprintf(stderr, "error: application of %s\n", type_name[first->type]);
			return no_object();
	}
	
	object call;
	init_call(&call, proc, args, cont);
	
	return perform_call(&call);
}

object* eval_list(object* args, object* cont) {
	object* list;
	object* environment;
	delist_2(args, &list, &environment);
	
	if (is_empty_list(list)) {
		object message;
		init_string(&message, "eval of empty list\n");
		object e;
		init_internal_error(&e, &message);
		return call_cont(cont, &e);
	}
	else {
		object next_ls[2];
		init_list_2(next_ls, list_rest(list), environment);
		object next_call;
		init_call(&next_call, &eval_list_rest_proc, next_ls, cont);
		object next_cont;
		init_cont(&next_cont, &next_call);
		
		object ls[2];
		init_list_2(ls, list_first(list), environment);
		object call;
		init_call(&call, eval_proc(), ls, &next_cont);
		
		return perform_call(&call);
	}
}

object* eval_symbol(object* args, object* cont) {
	object* obj;
	object* environment;
	delist_2(args, &obj, &environment);
	
	object* value = find_in_environment(environment, obj);
	
	return call_cont(cont, value);
}

object* eval_with_environment(object* args, object* cont) {
	object* environment;
	object* body;
	delist_2(args, &environment, &body);
	
	object ls[2];
	init_list_2(ls, body, environment);
	object call;
	init_call(&call, eval_proc(), ls, cont);
	
	return perform_call(&call);
}

object* eval(object* args, object* cont) {
	object* obj;
	object* environment;
	delist_2(args, &obj, &environment);
	
	object* proc;
	
	switch (obj->type) {
		case type_symbol:
			proc = &eval_symbol_proc;
			break;
		case type_list:
			proc = &eval_list_proc;
			break;
		default:
			proc = &eval_identity_proc;
			break;
	}
	object call;
	init_call(&call, proc, args, cont);
	
	return perform_call(&call);
}

void init_eval_procedures(void) {
	init_primitive_procedure(eval_proc(), &eval);
	init_primitive_procedure(eval_with_environment_proc(), &eval_with_environment);
	init_primitive_procedure(&eval_identity_proc, &eval_identity);
	init_primitive_procedure(&eval_symbol_proc, &eval_symbol);
	init_primitive_procedure(&eval_list_proc, &eval_list);
	init_primitive_procedure(&eval_list_rest_proc, &eval_list_rest);
	init_primitive_procedure(&eval_syntax_proc, &eval_syntax);
	init_primitive_procedure(&eval_primitive_procedure_proc, &eval_primitive_procedure);
	init_primitive_procedure(&eval_primitive_procedure_call_proc, &eval_primitive_procedure_call);
	
	init_primitive_procedure(&eval_function_proc, &eval_function);
	init_primitive_procedure(&eval_function_body_proc, &eval_function_body);
	
	init_primitive_procedure(eval_list_elements_proc(), &eval_list_elements);
	init_primitive_procedure(&eval_list_elements_first_proc, &eval_list_elements_first);
	init_primitive_procedure(&eval_list_elements_rest_proc, &eval_list_elements_rest);
}
