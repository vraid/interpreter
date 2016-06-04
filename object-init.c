#include <string.h>

#include "object-init.h"
#include "global-variables.h"

void init_object(object_location loc, object_type t, object* obj) {
	obj->type = t;
	obj->location = loc;
}

void init_string(object* obj, char* value) {
	init_object(location_stack, type_string, obj);
	obj->data.string.value = value;
	obj->data.string.length = strlen(value);
}

void init_symbol(object* obj, object* name) {
	init_object(location_stack, type_symbol, obj);
	obj->data.symbol.name = name;
}

void init_quote(object* obj, object* value) {
	init_object(location_stack, type_quote, obj);
	obj->data.quote.value = value;
}

void init_binding(object* obj, object* name, object* value) {
	init_object(location_stack, type_binding, obj);
	obj->data.binding.name = name;
	obj->data.binding.value = value;
}

void init_environment(object* obj, object* bindings) {
	init_object(location_stack, type_environment, obj);
	obj->data.environment.bindings = bindings;
}

void init_function(object* obj, object* environment, object* parameters, object* body) {
	init_object(location_stack, type_function, obj);
	obj->data.function.environment = environment;
	obj->data.function.parameters = parameters;
	obj->data.function.body = body;
}

void init_primitive_procedure(object* obj, primitive_proc* proc) {
	init_object(location_static, type_primitive_procedure, obj);
	obj->data.primitive_procedure.proc = proc;
}

void init_call(object* obj, object* function, object* arguments, object* continuation) {
	init_object(location_stack, type_call, obj);
	obj->data.call.function = function;
	obj->data.call.arguments = arguments;
	obj->data.call.continuation = continuation;
}

void init_cont_base(object* obj, object* call, char discard) {
	init_object(location_stack, type_continuation, obj);
	obj->data.continuation.discard_argument = discard;
	obj->data.continuation.call = call;
}

void init_cont(object* obj, object* call) {
	init_cont_base(obj, call, 0);
}

void init_discarding_cont(object* obj, object* call) {
	init_cont_base(obj, call, 1);
}

void init_internal_error(object* obj, object* message) {
	init_object(location_stack, type_internal_error, obj);
	obj->data.internal_error.message = message;
}

void init_list_cell(object* obj, object* first, object* rest) {
	init_object(location_stack, type_list, obj);
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
