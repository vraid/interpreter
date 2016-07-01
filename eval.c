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

char invalid_value_string[] = "invalid value: ";

object* eval_invalid_value(object* string, object* cont) {
	char* str = alloca(sizeof(char) * (1 + strlen(invalid_value_string) + string_length(string)));
	strcpy(str, invalid_value_string);
	strcpy(str + strlen(invalid_value_string), string_value(string));
	return throw_error(cont, str);
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
	init_call(&call, &eval_list_elements_proc, ls, &proc_cont);
	
	return perform_call(&call);
}

object* eval_function_call(object* args, object* cont) {
	object* arguments;
	object* function;
	delist_2(args, &arguments, &function);
	
	object eval_args[1];
	init_list_1(eval_args, function_body(function));
	object eval_call;
	init_call(&eval_call, &eval_with_environment_proc, eval_args, cont);
	object eval_cont;
	init_cont(&eval_cont, &eval_call);
	
	object bind_args[3];
	init_list_3(bind_args, arguments, function_parameters(function), function_environment(function));
	object bind_call;
	init_call(&bind_call, &bind_values_proc, bind_args, &eval_cont);
	
	return perform_call(&bind_call);
}

object eval_function_proc;

object* eval_function(object* args, object* cont) {
	object* function;
	object* arguments;
	object* environment;
	delist_3(args, &function, &arguments, &environment);
	
	object call_args[1];
	init_list_1(call_args, function);
	object call;
	init_call(&call, &eval_function_call_proc, call_args, cont);
	object next_cont;
	init_cont(&next_cont, &call);
	
	object eval_args[2];
	init_list_2(eval_args, arguments, environment);
	object eval_call;
	init_call(&eval_call, &eval_list_elements_proc, eval_args, &next_cont);
	
	return perform_call(&eval_call);
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
		object build_args[2];
		init_list_2(build_args, list_rest(unevaled), environment);
		object build_call;
		init_call(&build_call, &eval_list_elements_rest_proc, build_args, cont);
		object build_cont;
		init_cont(&build_cont, &build_call);

		object add_args[1];
		init_list_1(add_args, last);
		object add_call;
		init_call(&add_call, &add_to_list_proc, add_args, &build_cont);
		object add_cont;
		init_cont(&add_cont, &add_call);
		
		object eval_args[2];
		init_list_2(eval_args, list_first(unevaled), environment);
		object eval_call;
		init_call(&eval_call, &eval_proc, eval_args, &add_cont);
		return perform_call(&eval_call);
	}
}

object eval_list_elements_first_proc;

object* eval_list_elements_first(object* args, object* cont) {
	object* elements;
	object* environment;
	delist_2(args, &elements, &environment);
	
	object rest_args[2];
	init_list_2(rest_args, list_rest(elements), environment);
	
	object make_args[2];
	init_list_2(make_args, &eval_list_elements_rest_proc, rest_args);
	object make_call;
	init_call(&make_call, &make_list_proc, make_args, cont);
	object make_cont;
	init_cont(&make_cont, &make_call);
	
	object eval_args[2];
	init_list_2(eval_args, list_first(elements), environment);
	object eval_call;
	init_call(&eval_call, &eval_proc, eval_args, &make_cont);
	
	return perform_call(&eval_call);
}

object* eval_list_elements(object* args, object* cont) {
	object* elements;
	object* environment;
	delist_2(args, &elements, &environment);
	
	if (is_empty_list(elements)) {
		return call_cont(cont, empty_list());
	}
	else {
		object call;
		init_call(&call, &eval_list_elements_first_proc, args, cont);
		
		return perform_call(&call);
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
			fprintf(stderr, "application of %s\n", object_type_name(first));
			return throw_error(cont, "application of non-function");
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
		return throw_error(cont, "eval of empty list");
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
		init_call(&call, &eval_proc, ls, &next_cont);
		
		return perform_call(&call);
	}
}

char unbound_variable_string[] = "unbound variable: ";

object* eval_symbol(object* args, object* cont) {
	object* obj;
	object* environment;
	delist_2(args, &obj, &environment);
	
	object* binding = find_in_environment(environment, obj, 0);
	
	if (is_no_binding(binding)) {
		object* string = symbol_name(obj);
		char* str = alloca(sizeof(char) * (1 + strlen(unbound_variable_string) + string_length(string)));
		strcpy(str, unbound_variable_string);
		strcpy(str + strlen(unbound_variable_string), string_value(string));
		return throw_error(cont, str);
	}
	else {
		return call_cont(cont, binding_value(binding));
	}
}

object* eval_with_environment(object* args, object* cont) {
	object* environment;
	object* body;
	delist_2(args, &environment, &body);
	
	object ls[2];
	init_list_2(ls, body, environment);
	object call;
	init_call(&call, &eval_proc, ls, cont);
	
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
	init_primitive_procedure(&eval_proc, &eval);
	init_primitive_procedure(&eval_with_environment_proc, &eval_with_environment);
	init_primitive_procedure(&eval_identity_proc, &eval_identity);
	init_primitive_procedure(&eval_symbol_proc, &eval_symbol);
	init_primitive_procedure(&eval_list_proc, &eval_list);
	init_primitive_procedure(&eval_list_rest_proc, &eval_list_rest);
	init_primitive_procedure(&eval_syntax_proc, &eval_syntax);
	init_primitive_procedure(&eval_primitive_procedure_proc, &eval_primitive_procedure);
	init_primitive_procedure(&eval_primitive_procedure_call_proc, &eval_primitive_procedure_call);
	
	init_primitive_procedure(&eval_function_proc, &eval_function);
	init_primitive_procedure(&eval_function_call_proc, &eval_function_call);
	
	init_primitive_procedure(&eval_list_elements_proc, &eval_list_elements);
	init_primitive_procedure(&eval_list_elements_first_proc, &eval_list_elements_first);
	init_primitive_procedure(&eval_list_elements_rest_proc, &eval_list_elements_rest);
}
