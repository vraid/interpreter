#include "syntax-validate.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "delist.h"
#include "list-util.h"
#include "call.h"
#include "environments.h"
#include "higher-order.h"
#include "generic-arguments.h"
#include "print.h"

object* throw_length_error(object* cont) {
	return throw_error_string(cont, "wrong length");
}

char is_symbol_list(object* ls) {
	if (!is_list(ls)) {
		return 0;
	}
	while (!is_empty_list(ls)) {
		if (!is_symbol(desyntax(list_first(ls)))) {
			return 0;
		}
		ls = list_rest(ls);
	}
	return 1;
}

object* invalid_function_signature(object* ls) {
	if (is_symbol(ls)) {
		return false();
	}
	else if (!is_nonempty_list(ls)) {
		return true();
	}
	object* a;
	while (is_list(ls)) {
		a = list_first(ls);
		object* rest = list_rest(ls);
		if (is_empty_list(a) || !is_symbol_list(rest)) {
			return true();
		}
		object* dup = find_duplicate(rest);
		if (dup != false()) {
			return dup;
		}
		ls = a;
	}
	return is_symbol(desyntax(a)) ? false() : true();
}

char list_has_width(int width, object* ls) {
	while (!is_empty_list(ls)) {
		object* a = desyntax(list_first(ls));
		if (!(is_list(a) && list_length(a) == width)) {
			return 0;
		}
		ls = list_rest(ls);
	}
	return 1;
}

int syntax_length[syntax_count];

void init_syntax_lengths() {
	syntax_length[syntax_define] = 2;
	syntax_length[syntax_quote] = 1;
	syntax_length[syntax_delay] = 1;
	syntax_length[syntax_force] = 1;
	syntax_length[syntax_let] = 2;	
	syntax_length[syntax_letrec] = 2;
	syntax_length[syntax_rec] = 3;
	syntax_length[syntax_lambda] = 2;
	syntax_length[syntax_curry] = 1;
	syntax_length[syntax_apply] = 0;
	syntax_length[syntax_if] = 3;
	syntax_length[syntax_and] = 0;
	syntax_length[syntax_or] = 0;
	syntax_length[syntax_struct] = 0;
	syntax_length[syntax_list] = 0;
	syntax_length[syntax_stream] = 2;
	syntax_length[syntax_vector] = 0;
	syntax_length[syntax_map] = 0;
	syntax_length[syntax_fold] = 0;
	syntax_length[syntax_filter] = 2;
}

object validate_list_elements_proc;

object* no_validate(object* args, object* cont) {
	object* stx;
	object* env;
	delist_2(args, &stx, &env);
	
	return call_cont(cont, stx);
}

object* standard_validate(object* args, object* cont) {
	object* stx;
	object* env;
	delist_2(args, &stx, &env);
	
	object* rest = list_rest(stx);
	object* call_args = alloc_list_2(rest, env);
	object* call = alloc_call(&validate_list_elements_proc, call_args, cont);
	return perform_call(call);
}

object* validate_define(object* args, object* cont) {
	object* stx;
	object* env;
	delist_2(args, &stx, &env);
	
	object* signature;
	object* value;
	delist_desyntax_2(list_rest(stx), &signature, &value);
	
	if (invalid_function_signature(signature) != false()) {
		object* str = alloc_string("invalid definition signature");
		object* ls = alloc_list_2(str, stx);
		return throw_error(cont, ls);
	}
	
	object* call_args = alloc_list_2(value, env);
	object* call = alloc_call(&validate_expression_proc, call_args, cont);
	return perform_call(call);
}

object* validate_lambda(object* args, object* cont) {
	object* stx;
	object* env;
	delist_2(args, &stx, &env);
	
	object* param;
	object* body;
	delist_desyntax_2(list_rest(stx), &param, &body);
	
	if (!is_symbol_list(param)) {
		object* str = alloc_string("parameters must be list of symbols");
		object* ls = alloc_list_2(str, param);
		return throw_error(cont, ls);
	}
	object* dup = find_duplicate(param);
	if (!is_false(dup)) {
		object* str = alloc_string("duplicate parameter");
		object* ls = alloc_list_2(str, param);
		return throw_error(cont, ls);
	}
	
	object* call_args = alloc_list_2(body, env);
	object* call = alloc_call(&validate_expression_proc, call_args, cont);
	return perform_call(call);
}

object validate_let_bindings_proc;

object* validate_let_bindings(object* args, object* cont) {
	object* stx;
	object* bindings;
	object* env;
	delist_3(args, &stx, &bindings, &env);
	
	if (is_empty_list(bindings)) {
		return call_discarding_cont(cont);
	}
	object* first = desyntax(list_first(bindings));
	object* rest = list_rest(bindings);
	
	object* value = desyntax(list_ref(1, first));
	
	object* next_args = alloc_list_2(rest, env);
	object* next_call = alloc_call(&validate_let_bindings_proc, next_args, cont);
	object* next_cont = alloc_cont(next_call);
	
	object* call_args = alloc_list_2(value, env);
	object* call = alloc_call(&validate_expression_proc, call_args, next_cont);
	
	return perform_call(call);
}

object* validate_let(object* args, object* cont) {
	object* stx;
	object* env;
	delist_2(args, &stx, &env);
	
	object* bindings;
	object* body;
	delist_desyntax_2(list_rest(stx), &bindings, &body);
	
	if (!(is_list(bindings) && list_has_width(2, bindings))) {
		object* str = alloc_string("malformed let bindings");
		object* ls = alloc_list_2(str, bindings);
		return throw_error(cont, ls);
	}
	
	object* next_args = alloc_list_2(body, env);
	object* next_call = alloc_call(&validate_expression_proc, next_args, cont);
	object* next_cont = alloc_discarding_cont(next_call);
	
	object* call_args = alloc_list_3(stx, bindings, env);
	object* call = alloc_call(&validate_let_bindings_proc, call_args, next_cont);
	
	return perform_call(call);
}

object validate_letrec_two_proc;

object* validate_letrec_two(object* args, object* cont) {
	object* names_values;
	object* stx;
	object* env;
	delist_3(args, &names_values, &stx, &env);
	
	object* names;
	object* values;
	delist_2(names_values, &names, &values);
	
	object* dup = find_duplicate(names);
	if (dup != false()) {
		object* str = alloc_string("duplicate binding in letrec");
		object* ls = alloc_list_2(str, stx);
		return throw_error(cont, ls);
	}
	
	object* bindings;
	object* body;
	delist_desyntax_2(list_rest(stx), &bindings, &body);
	
	object* next_args = alloc_list_2(body, env);
	object* next_call = alloc_call(&validate_expression_proc, next_args, cont);
	object* next_cont = alloc_discarding_cont(next_call);
	
	object* call_args = alloc_list_3(stx, bindings, env);
	object* call = alloc_call(&validate_let_bindings_proc, call_args, next_cont);
	
	return perform_call(call);
}

object* validate_letrec(object* args, object* cont) {
	object* stx;
	object* env;
	delist_2(args, &stx, &env);
	
	object* bindings;
	object* body;
	delist_desyntax_2(list_rest(stx), &bindings, &body);
	
	if (!(is_list(bindings) && list_has_width(2, bindings))) {
		object* str = alloc_string("malformed letrec bindings");
		object* ls = alloc_list_2(str, stx);
		return throw_error(cont, ls);
	}
	
	object* next_call = alloc_call(&validate_letrec_two_proc, args, cont);
	object* next_cont = alloc_cont(next_call);
	
	object* unzip_args = alloc_list_1(bindings);
	object* unzip_call = alloc_call(&unzip_2_proc, unzip_args, next_cont);
	
	return perform_call(unzip_call);
}

object* validate_rec(object* args, object* cont) {
	object* stx;
	object* env;
	delist_2(args, &stx, &env);
	
	object* name;
	object* bindings;
	object* body;
	delist_desyntax_3(list_rest(stx), &name, &bindings, &body);
	
	if (!is_symbol(name)) {
		object* str = alloc_string("rec binding must be symbol");
		object* ls = alloc_list_2(str, stx);
		return throw_error(cont, ls);
	}
	if (!(is_list(bindings) && list_has_width(2, bindings))) {
		object* str = alloc_string("malformed rec bindings");
		object* ls = alloc_list_2(str, bindings);
		return throw_error(cont, ls);
	}
	
	object* next_args = alloc_list_2(body, env);
	object* next_call = alloc_call(&validate_expression_proc, next_args, cont);
	object* next_cont = alloc_discarding_cont(next_call);
	
	object* binding_args = alloc_list_3(stx, bindings, env);
	object* binding_call = alloc_call(&validate_let_bindings_proc, binding_args, next_cont);
	
	return perform_call(binding_call);
}

object* validate_struct(object* args, object* cont) {
	object* stx;
	object* env;
	delist_2(args, &stx, &env);
	
	int n = list_length(stx)-1;
	if (n < 2 || n > 3) {
		return throw_length_error(cont);
	}
	
	object* name;
	object* parent = no_symbol();
	object* fields;
	
	if (n == 3) {
		delist_desyntax_3(list_rest(stx), &name, &parent, &fields);
	}
	else {
		delist_desyntax_2(list_rest(stx), &name, &fields);
	}
	
	if (!is_symbol(name)) {
		object* str = alloc_string("invalid struct name");
		object* ls = alloc_list_3(str, name, stx);
		return throw_error(cont, ls);
	}
	if (!is_symbol(parent)) {
		object* str = alloc_string("invalid struct parent");
		object* ls = alloc_list_3(str, parent, stx);
		return throw_error(cont, ls);
	}
	if (!is_symbol_list(fields)) {
		object* str = alloc_string("invalid struct fields");
		object* ls = alloc_list_2(str, stx);
		return throw_error(cont, ls);
	}
	object* dup = find_duplicate(fields);
	if (dup != false()) {
		object* str = alloc_string("duplicate struct field");
		object* ls = alloc_list_3(str, dup, stx);
		return throw_error(cont, ls);
	}
	return call_cont(cont, stx);
}

object syntax_validate[syntax_count];

object* validate_list_elements(object* args, object* cont) {
	object* stx;
	object* env;
	delist_2(args, &stx, &env);
	
	object* body = alloc_list_3(&validate_expression_proc, generic_args[0], env);
	object* func = alloc_function(empty_environment(), generic_arg_list[1], body);
	
	object* map_list = alloc_list_2(func, stx);
	object* map_args = alloc_list_1(map_list);
	object* map_call = alloc_call(&map_proc, map_args, cont);
	
	return perform_call(map_call);
}

object validate_list_proc;

object* validate_list(object* args, object* cont) {
	object* stx;
	object* env;
	delist_2(args, &stx, &env);
	
	if (is_empty_list(stx)) {
		return throw_error_string(cont, "expression cannot be empty list");
	}
	else {
		object* obj = desyntax(list_first(stx));
		if (is_symbol(obj)) {
			object* a = find_in_environment(env, obj, 1);
			if (!is_no_binding(a)) {
				object* val = binding_value(a);
				if (is_syntax_procedure(val)) {
					static_syntax_procedure id = syntax_procedure_id(val);	
					int n = list_length(stx);
					if (syntax_length[id] > 0 && syntax_length[id]+1 != n) {
						return throw_length_error(cont);
					}
					object* call = alloc_call(&syntax_validate[id], args, cont);
					return perform_call(call);
				}
			}
		}
		object* list_call = alloc_call(&validate_list_elements_proc, args, cont);
		return perform_call(list_call);
	}
}

object validate_atom_proc;

object* validate_atom(object* args, object* cont) {
	object* atom;
	object* env;
	delist_2(args, &atom, &env);
	
	return call_cont(cont, atom);
}

object return_syntax_proc;

object* return_syntax(object* args, object* cont) {
	object* throw;
	object* stx;
	delist_2(args, &throw, &stx);
	
	return call_cont(cont, stx);
}	

object* validate_expression(object* args, object* cont) {
	object* stx;
	object* env;
	delist_2(args, &stx, &env);
	
	object* obj = desyntax(stx);

	object* return_args = alloc_list_1(stx);	
	object* return_call = alloc_call(&return_syntax_proc, return_args, cont);
	object* return_cont = alloc_cont(return_call);
	
	object* call_args = alloc_list_2(obj, env);
	object* call = alloc_call(is_list(obj) ? &validate_list_proc : &validate_atom_proc, call_args, return_cont);
	
	return perform_call(call);
}

void init_validate_procedures(void) {
	init_syntax_lengths();
	int i;
	for (i = 0; i < syntax_count; i++) {
		init_primitive_procedure(&syntax_validate[i], &standard_validate);
	}
	
	init_primitive_procedure(&syntax_validate[syntax_define], &validate_define);
	init_primitive_procedure(&syntax_validate[syntax_quote], &no_validate);
	init_primitive_procedure(&syntax_validate[syntax_lambda], &validate_lambda);
	init_primitive_procedure(&syntax_validate[syntax_let], &validate_let);
	init_primitive_procedure(&syntax_validate[syntax_letrec], &validate_letrec);
	init_primitive_procedure(&syntax_validate[syntax_rec], &validate_rec);
	init_primitive_procedure(&syntax_validate[syntax_struct], &validate_struct);
	
	init_primitive(&validate_let_bindings, &validate_let_bindings_proc);
	init_primitive(&validate_letrec_two, &validate_letrec_two_proc);
	init_primitive(&validate_expression, &validate_expression_proc);
	init_primitive(&return_syntax, &return_syntax_proc);
	init_primitive(&validate_atom, &validate_atom_proc);
	init_primitive(&validate_list, &validate_list_proc);
	init_primitive(&validate_list_elements, &validate_list_elements_proc);
}
