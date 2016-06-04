#include "eval.h"

#include <stdlib.h>
#include <string.h>
#include "data-structures.h"
#include "object-init.h"
#include "global-variables.h"
#include "call.h"
#include "delist.h"

object eval_atom_proc;
object eval_function_proc;
object eval_call_proc;
object eval_proc;

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

object* eval_hash_literal(object* args, object* cont) {
	object* symbol;
	delist_1(args, &symbol);
	
	object* string = symbol_name(symbol);
	
	if (string_length(string) == 2) {
		switch (string_value(string)[1]) {
			case 't' : return call_cont(cont, true());
			case 'f' : return call_cont(cont, false());
		}
	}

	return eval_invalid_value(string, cont);
}

object* eval_atom(object* args, object* cont) {
	object* symbol;
	delist_1(args, &symbol);
	
	object* string = symbol_name(symbol);
	object* value = no_object();
	
	if (string_value(string)[0] == '#') {
		return eval_hash_literal(args, cont);
	}
	else if (is_no_object(value)) {
		return eval_invalid_value(string, cont);
	}
	else {
		return call_cont(cont, value);
	}
}

object* eval_function(object* args, object* cont) {
	return call_cont(cont, args);
}

object* eval_call(object* args, object* cont) {
	return call_cont(cont, args);
}

object* eval(object* args, object* cont) {
	object* obj;
	delist_1(args, &obj);
	
	object* proc;

	object call;
	object ls[1];
	init_list_1(ls, obj);
	
	switch (obj->type) {
		case type_call:
			proc = &eval_call_proc;
			break;
		default:
			proc = &eval_atom_proc;
			break;
	}
	init_call(&call, proc, ls, cont);
	
	return perform_call(&call);
}

void init_eval_procedures(void) {
	init_primitive_procedure(&eval_atom_proc, &eval_atom);
	init_primitive_procedure(&eval_function_proc, &eval_function);
	init_primitive_procedure(&eval_call_proc, &eval_call);
	init_primitive_procedure(&eval_proc, &eval);
}
