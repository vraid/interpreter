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
#include "base-util.h"
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
	ls = desyntax(ls);
	if (is_symbol(ls)) {
		return false();
	}
	else if (!is_nonempty_list(ls)) {
		return true();
	}
	object* a;
	while (is_list(ls)) {
		a = desyntax(list_first(ls));
		object* rest = desyntax(list_rest(ls));
		if (is_empty_list(a) || !is_symbol_list(rest)) {
			return true();
		}
		object* dup = find_duplicate(rest);
		if (dup != false()) {
			return dup;
		}
		ls = a;
	}
	return is_symbol(a) ? false() : true();
}

char is_symbol_binding_list(object* ls) {
	while (!is_empty_list(ls)) {
		object* a = desyntax(list_first(ls));
		if (!is_binding(a) || !is_symbol(desyntax(binding_name(a)))) {
			return 0;
		}
		ls = list_rest(ls);
	}
	return 1;
}

int syntax_length[syntax_count];

void init_syntax_lengths() {
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
	syntax_length[syntax_list] = 0;
	syntax_length[syntax_stream] = 2;
	syntax_length[syntax_vector] = 0;
	syntax_length[syntax_map] = 0;
	syntax_length[syntax_fold] = 0;
	syntax_length[syntax_filter] = 2;
}

object return_syntax_proc;

object validate_list_elements_proc;

object* no_validate(object* args, object* cont) {
	object* stx;
	object* env;
	object* trace;
	delist_3(args, &stx, &env, &trace);
	
	return call_cont(cont, stx);
}

object* standard_validate(object* args, object* cont) {
	object* stx;
	object* env;
	object* trace;
	delist_3(args, &stx, &env, &trace);
	
	object* rest = list_rest(stx);
	object* call_args = alloc_list_3(rest, env, trace);
	object* call = alloc_call(&validate_list_elements_proc, call_args, cont);
	return perform_call(call);
}

object* validate_define(object* args, object* cont) {
	object* stx;
	object* env;
	object* trace;
	delist_3(args, &stx, &env, &trace);
	
	object* signature;
	object* value;
	delist_desyntax_2(list_rest(stx), &signature, &value);
	
	if (invalid_function_signature(signature) != false()) {
		object* str = alloc_string("invalid definition signature");
		object* ls = alloc_list_2(str, stx);
		return throw_error(cont, ls);
	}
	
	object* call_args = alloc_list_3(value, env, trace);
	object* call = alloc_call(&validate_expression_proc, call_args, cont);
	return perform_call(call);
}

object* validate_lambda(object* args, object* cont) {
	object* stx;
	object* env;
	object* trace;
	delist_3(args, &stx, &env, &trace);
	
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
	
	object* call_args = alloc_list_3(body, env, trace);
	object* call = alloc_call(&validate_expression_proc, call_args, cont);
	return perform_call(call);
}

object binding_names_proc;

object* binding_names(object* args, object* cont) {
	object* bindings;
	delist_1(args, &bindings);
	
	object* names = empty_list();
	while (!is_empty_list(bindings)) {
		object* first = desyntax(list_first(bindings));
		names = alloc_list_cell(desyntax(binding_name(first)), names);
		
		bindings = list_rest(bindings);
	}
	
	object* reverse_call = alloc_call(&reverse_list_proc, alloc_list_1(names), cont);
	return perform_call(reverse_call);
}

object validate_let_bindings_proc;

object* validate_let_bindings(object* args, object* cont) {
	object* stx;
	object* bindings;
	object* env;
	object* trace;
	delist_4(args, &stx, &bindings, &env, &trace);
	
	if (is_empty_list(bindings)) {
		return call_discarding_cont(cont);
	}
	object* first = desyntax(list_first(bindings));
	object* rest = list_rest(bindings);
	
	object* value = desyntax(binding_value(first));
	
	object* next_args = alloc_list_3(rest, env, trace);
	object* next_call = alloc_call(&validate_let_bindings_proc, next_args, cont);
	
	object* call_args = alloc_list_3(value, env, trace);
	object* call = alloc_call(&validate_expression_proc, call_args, alloc_cont(next_call));
	
	return perform_call(call);
}

object* validate_let(object* args, object* cont) {
	object* stx;
	object* env;
	object* trace;
	delist_3(args, &stx, &env, &trace);
	
	object* bindings;
	object* body;
	delist_desyntax_2(list_rest(stx), &bindings, &body);
	
	if (!(is_list(bindings) && is_symbol_binding_list(bindings))) {
		object* str = alloc_string("malformed let bindings");
		object* ls = alloc_list_2(str, bindings);
		return throw_error(cont, ls);
	}
	
	object* next_args = alloc_list_3(body, env, trace);
	object* next_call = alloc_call(&validate_expression_proc, next_args, cont);
	object* next_cont = alloc_discarding_cont(next_call);
	
	object* call_args = alloc_list_4(stx, bindings, env, trace);
	object* call = alloc_call(&validate_let_bindings_proc, call_args, next_cont);
	
	return perform_call(call);
}

object validate_letrec_two_proc;

object* validate_letrec_two(object* args, object* cont) {
	object* names;
	object* stx;
	object* env;
	object* trace;
	delist_4(args, &names, &stx, &env, &trace);
	
	object* dup = find_duplicate(names);
	if (dup != false()) {
		object* str = alloc_string("duplicate binding in letrec");
		object* ls = alloc_list_2(str, stx);
		return throw_error(cont, ls);
	}
	
	object* bindings;
	object* body;
	delist_desyntax_2(list_rest(stx), &bindings, &body);
	
	object* next_args = alloc_list_3(body, env, trace);
	object* next_call = alloc_call(&validate_expression_proc, next_args, cont);
	object* next_cont = alloc_discarding_cont(next_call);
	
	object* call_args = alloc_list_4(stx, bindings, env, trace);
	object* call = alloc_call(&validate_let_bindings_proc, call_args, next_cont);
	
	return perform_call(call);
}

object* validate_letrec(object* args, object* cont) {
	object* stx;
	object* env;
	object* trace;
	delist_3(args, &stx, &env, &trace);
	
	object* bindings;
	object* body;
	delist_desyntax_2(list_rest(stx), &bindings, &body);
	
	if (!(is_list(bindings) && is_symbol_binding_list(bindings))) {
		object* str = alloc_string("malformed letrec bindings");
		object* ls = alloc_list_2(str, stx);
		return throw_error(cont, ls);
	}
	
	object* next_call = alloc_call(&validate_letrec_two_proc, args, cont);
	object* names_call = alloc_call(&binding_names_proc, alloc_list_1(bindings), alloc_cont(next_call));
	
	return perform_call(names_call);
}

object* validate_rec(object* args, object* cont) {
	object* stx;
	object* env;
	object* trace;
	delist_3(args, &stx, &env, &trace);
	
	object* name;
	object* bindings;
	object* body;
	delist_desyntax_3(list_rest(stx), &name, &bindings, &body);
	
	if (!is_symbol(name)) {
		object* str = alloc_string("rec binding must be symbol");
		object* ls = alloc_list_2(str, stx);
		return throw_error(cont, ls);
	}
	if (!(is_list(bindings) && is_symbol_binding_list(bindings))) {
		object* str = alloc_string("malformed rec bindings");
		object* ls = alloc_list_2(str, bindings);
		return throw_error(cont, ls);
	}
	
	object* next_args = alloc_list_3(body, env, trace);
	object* next_call = alloc_call(&validate_expression_proc, next_args, cont);
	object* next_cont = alloc_discarding_cont(next_call);
	
	object* binding_args = alloc_list_4(stx, bindings, env, trace);
	object* binding_call = alloc_call(&validate_let_bindings_proc, binding_args, next_cont);
	
	return perform_call(binding_call);
}

object syntax_validate[syntax_count];

object validate_list_element_proc;

object* validate_list_element(object* args, object* cont) {
	object* elem;
	object* elements;
	object* env;
	object* trace;
	delist_4(args, &elem, &elements, &env, &trace);
	
	if (is_empty_list(elements)) {
		return call_discarding_cont(cont);
	}
	else {
		object* next_args = alloc_list_3(list_rest(elements), env, trace);
		object* next_call = alloc_call(&validate_list_element_proc, next_args, cont);
		
		object* validate_args = alloc_list_3(list_first(elements), env, trace);
		object* validate_call = alloc_call(&validate_expression_proc, validate_args, alloc_cont(next_call));
		
		return perform_call(validate_call);
	}
}

object* validate_list_elements(object* args, object* cont) {
	object* stx;
	object* env;
	object* trace;
	delist_3(args, &stx, &env, &trace);
	
	object* return_args = alloc_list_1(stx);
	object* return_call = alloc_call(&identity_proc, return_args, cont);
	object* return_cont = alloc_discarding_cont(return_call);
	
	object* validate_args = alloc_list_4(no_object(), desyntax(stx), env, trace);
	object* validate_call = alloc_call(&validate_list_element_proc, validate_args, return_cont);
	
	return perform_call(validate_call);
}

object validate_list_proc;

object* validate_list(object* args, object* cont) {
	object* stx;
	object* env;
	object* trace;
	delist_3(args, &stx, &env, &trace);
	
	stx = desyntax(stx);
	
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
					object* validate_args = alloc_list_3(stx, env, trace);
					object* call = alloc_call(&syntax_validate[id], validate_args, cont);
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
	object* trace;
	delist_3(args, &atom, &env, &trace);
	
	return call_cont(cont, atom);
}

object* return_syntax(object* args, object* cont) {
	object* throw;
	object* stx;
	delist_2(args, &throw, &stx);
	
	return call_cont(cont, stx);
}	

object* validate_expression(object* args, object* cont) {
	object* stx;
	object* env;
	object* trace;
	delist_3(args, &stx, &env, &trace);
	
	object* obj = desyntax(stx);

	object* return_args = alloc_list_1(stx);	
	object* return_call = alloc_call(&return_syntax_proc, return_args, cont);
	
	object* call_args = alloc_list_3(obj, env, trace);
	object* call = alloc_call(is_list(obj) ? &validate_list_proc : &validate_atom_proc, call_args, alloc_cont(return_call));
	
	return perform_call(call);
}

void init_validate_procedures(void) {
	init_syntax_lengths();
	int i;
	for (i = 0; i < syntax_count; i++) {
		init_primitive_procedure(&syntax_validate[i], &standard_validate);
	}
	
	init_primitive_procedure(&syntax_validate[syntax_quote], &no_validate);
	init_primitive_procedure(&syntax_validate[syntax_lambda], &validate_lambda);
	init_primitive_procedure(&syntax_validate[syntax_let], &validate_let);
	init_primitive_procedure(&syntax_validate[syntax_letrec], &validate_letrec);
	init_primitive_procedure(&syntax_validate[syntax_rec], &validate_rec);
	
	init_primitive(&binding_names, &binding_names_proc);
	init_primitive(&validate_let_bindings, &validate_let_bindings_proc);
	init_primitive(&validate_letrec_two, &validate_letrec_two_proc);
	init_primitive(&validate_expression, &validate_expression_proc);
	init_primitive(&return_syntax, &return_syntax_proc);
	init_primitive(&validate_atom, &validate_atom_proc);
	init_primitive(&validate_list, &validate_list_proc);
	init_primitive(&validate_list_elements, &validate_list_elements_proc);
	init_primitive(&validate_list_element, &validate_list_element_proc);
}
