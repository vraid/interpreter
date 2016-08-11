#include <string.h>

#include "object-init.h"
#include "global-variables.h"

void make_static(object* obj) {
	obj->location = location_static;
}

void init_object(object_location loc, object_type t, object* obj) {
	obj->type = t;
	obj->location = loc;
}

void init_string(object* obj, char* value) {
	init_object(location_stack, type_string, obj);
	obj->data.string.value = value;
	obj->data.string.length = strlen(value);
}

void init_fixnum(object* obj, long value) {
	init_object(location_stack, type_fixnum, obj);
	obj->data.fixnum.value = value;
}

void init_integer(object* obj, int sign, object* digits) {
	init_object(location_stack, type_integer, obj);
	obj->data.integer.sign = sign;
	obj->data.integer.digits = digits;
}

void init_positive_integer(object* obj, object* digits) {
	init_integer(obj, 1, digits);
}

void init_symbol(object* obj, object* name) {
	init_object(location_stack, type_symbol, obj);
	obj->data.symbol.name = name;
}

void init_binding(object* obj, object* name, object* value) {
	init_object(location_stack, type_binding, obj);
	obj->data.binding.name = name;
	obj->data.binding.value = value;
}

void init_stream(object* obj, object* first, object* rest) {
	init_object(location_stack, type_stream, obj);
	obj->data.stream.first = first;
	obj->data.stream.rest = rest;
}

void init_vector(object* obj, int length, object** data) {
	init_object(location_stack, type_vector, obj);
	obj->data.vector.length = length;
	obj->data.vector.data = data;
}

void init_vector_iterator(object* obj, int n, object* vector) {
	init_object(location_stack, type_vector_iterator, obj);
	obj->data.vector_iterator.n = n;
	obj->data.vector_iterator.vector = vector;
}

void init_struct_definition(object* obj, object* name, object* fields, object* constructor, object* parent) {
	init_object(location_stack, type_struct_definition, obj);
	obj->data.struct_definition.name = name;
	obj->data.struct_definition.fields = fields;
	obj->data.struct_definition.constructor = constructor;
	obj->data.struct_definition.parent = parent;
}

void init_struct_instance(object* obj, object* type, object* data) {
	init_object(location_stack, type_struct_instance, obj);
	obj->data.struct_instance.type = type;
	obj->data.struct_instance.data = data;
}

void init_environment(object* obj, object* bindings) {
	init_object(location_stack, type_environment, obj);
	obj->data.environment.bindings = bindings;
}

void init_syntax(object* obj, primitive_proc* proc) {
	init_object(location_static, type_syntax, obj);
	obj->data.syntax.proc = proc;
}

void init_function(object* obj, object* environment, object* parameters, object* body) {
	init_object(location_stack, type_function, obj);
	obj->data.function.environment = environment;
	obj->data.function.parameters = parameters;
	obj->data.function.body = body;
}

void init_call(object* obj, object* function, object* arguments, object* continuation) {
	init_object(location_stack, type_call, obj);
	obj->data.call.function = function;
	obj->data.call.arguments = arguments;
	obj->data.call.continuation = continuation;
}

void init_cont_base(object* obj, object* call, cont_type t) {
	init_object(location_stack, type_continuation, obj);
	obj->data.continuation.conttype = t;
	obj->data.continuation.call = call;
}

void init_cont(object* obj, object* call) {
	init_cont_base(obj, call, cont_normal);
}

void init_discarding_cont(object* obj, object* call) {
	init_cont_base(obj, call, cont_discarding);
}

void init_catching_cont(object* obj, object* call) {
	init_cont_base(obj, call, cont_catching);
}

void init_internal_error(object* obj, object* message) {
	init_object(location_stack, type_internal_error, obj);
	obj->data.internal_error.message = message;
}

void init_delay(object* obj, object* value, object* environment) {
	init_object(location_stack, type_delay, obj);
	obj->data.delay.value = value;
	obj->data.delay.environment = environment;
	obj->data.delay.evaluated = 0;
}

void init_primitive_procedure(object* obj, primitive_proc* proc) {
	init_object(location_static, type_primitive_procedure, obj);
	obj->data.primitive_procedure.proc = proc;
}

void init_primitive(primitive_proc* proc, object* obj) {
	init_primitive_procedure(obj, proc);
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

void init_list_4(object* ls, object* first, object* second, object* third, object* fourth) {
	init_list_cell(ls, first, ls+1);
	init_list_3(ls+1, second, third, fourth);
}

void init_list_5(object* ls, object* first, object* second, object* third, object* fourth, object* fifth) {
	init_list_cell(ls, first, ls+1);
	init_list_4(ls+1, second, third, fourth, fifth);
}
