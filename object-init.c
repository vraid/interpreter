#include "object-init.h"
#include "global-variables.h"

void init_binding(object* obj, object* name, object* value) {
	obj->type = type_binding;
	obj->data.binding.name = name;
	obj->data.binding.value = value;
}

void init_environment(object* obj, object* bindings) {
	obj->type = type_environment;
	obj->data.environment.bindings = bindings;
}

void make_function(object* obj, object* environment, object* parameters, object* body) {
	obj->type = type_function;
	obj->data.function.environment = environment;
	obj->data.function.parameters = parameters;
	obj->data.function.body = body;
}

void init_primitive_procedure(object* obj, primitive_proc* proc) {
	obj->type = type_primitive_procedure;
	obj->data.primitive_procedure.proc = proc;
}

void init_call(object* obj, object* function, object* arguments, object* continuation) {
	obj->type = type_call;
	obj->data.call.function = function;
	obj->data.call.arguments = arguments;
	obj->data.call.continuation = continuation;
}

void init_cont_base(object* obj, object* call, char discard) {
	obj->type = type_continuation;
	obj->data.continuation.discard_argument = discard;
	obj->data.continuation.call = call;
}

void init_cont(object* obj, object* call) {
	init_cont_base(obj, call, 0);
}

void init_discarding_cont(object* obj, object* call) {
	init_cont_base(obj, call, 1);
}

void init_list_cell(object* obj, object* first, object* rest) {
	obj->type = type_list;
	obj->data.list.first = first;
	obj->data.list.rest = rest;
}

void init_list_1(object* ls, object* first) {
	init_list_cell(ls, first, empty_list());
}

void init_list_2(object* ls, object* first, object* second) {
	init_list_cell(ls, first, ls+1);
	init_list_1(ls+1, second);
}

void init_list_3(object* ls, object* first, object* second, object* third) {
	init_list_cell(ls, first, ls+1);
	init_list_2(ls+1, second, third);
}

/*
object* make_binding_list(object* names, object* values) {
	if (is_empty_list(names)) {
		return empty_list();
	}
	else {
		object* ls = new_list();
		object* prev;
		object* next = ls;
		while (!is_empty_list(names)) {
			prev = next;
			prev->data.list.first = make_binding(list_first(names), list_first(values));
		
			names = list_rest(names);
			values = list_rest(values);
		
			if (is_empty_list(names)) {
				next = empty_list();
			}
			else {
				next = new_list();
			}
			prev->data.list.rest = next;
		}
		return ls;
	}
}
*/
