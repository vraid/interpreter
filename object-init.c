#include <string.h>

#include "object-init.h"
#include "global-variables.h"

void make_static(object* obj) {
	obj->location = location_static;
}

void make_semistatic(object* obj) {
	obj->location = location_semistatic;
}

object* init_object(object_location loc, object_type t, object* obj) {
	obj->type = t;
	obj->location = loc;
	return obj;
}

object* init_string(object* obj, char* value) {
	init_object(location_stack, type_string, obj);
	obj->data.string.value = value;
	obj->data.string.length = strlen(value);
	return obj;
}

object* init_fixnum(object* obj, long value) {
	init_object(location_stack, type_fixnum, obj);
	obj->data.fixnum.value = value;
	return obj;
}

object* init_integer(object* obj, int sign, object* digits) {
	init_object(location_stack, type_integer, obj);
	obj->data.integer.sign = sign;
	obj->data.integer.digits = digits;
	return obj;
}

object* init_positive_integer(object* obj, object* digits) {
	init_integer(obj, 1, digits);
	return obj;
}

object* init_negated_integer(object* obj, object* num) {
	init_integer(obj, -1 * integer_sign(num), integer_digits(num));
	return obj;
}

object* init_fraction(object* obj, object* numerator, object* denominator) {
	init_object(location_stack, type_fraction, obj);
	obj->data.fraction.numerator = numerator;
	obj->data.fraction.denominator = denominator;
	return obj;
}

object* init_integral_fraction(object* obj, object* numerator) {
	init_fraction(obj, numerator, integer_one());
	return obj;
}

object* init_complex(object* obj, object* real, object* imag) {
	init_object(location_stack, type_complex, obj);
	obj->data.complex.real_part = real;
	obj->data.complex.imag_part = imag;
	return obj;
}

object* init_symbol(object* obj, object* name) {
	init_object(location_stack, type_symbol, obj);
	obj->data.symbol.name = name;
	return obj;
}

object* init_binding(object* obj, object* name, object* value) {
	init_object(location_stack, type_binding, obj);
	obj->data.binding.name = name;
	obj->data.binding.value = value;
	return obj;
}

object* init_stream(object* obj, object* first, object* rest) {
	init_object(location_stack, type_stream, obj);
	obj->data.stream.first = first;
	obj->data.stream.rest = rest;
	return obj;
}

object* init_vector(object* obj, int length, object** data) {
	init_object(location_stack, type_vector, obj);
	obj->data.vector.length = length;
	obj->data.vector.data = data;
	return obj;
}

object* init_vector_iterator(object* obj, int n, object* vector) {
	init_object(location_stack, type_vector_iterator, obj);
	obj->data.vector_iterator.n = n;
	obj->data.vector_iterator.vector = vector;
	return obj;
}

object* init_struct_definition(object* obj, object* name, object* fields, object* constructor, object* parent) {
	init_object(location_stack, type_struct_definition, obj);
	obj->data.struct_definition.name = name;
	obj->data.struct_definition.fields = fields;
	obj->data.struct_definition.constructor = constructor;
	obj->data.struct_definition.parent = parent;
	return obj;
}

object* init_struct_instance(object* obj, object* type, object* data) {
	init_object(location_stack, type_struct_instance, obj);
	obj->data.struct_instance.type = type;
	obj->data.struct_instance.data = data;
	return obj;
}

object* init_environment(object* obj, object* bindings) {
	init_object(location_stack, type_environment, obj);
	obj->data.environment.bindings = bindings;
	return obj;
}

object* init_syntax_procedure(object* obj, primitive_proc* proc, static_syntax_procedure id) {
	init_object(location_static, type_syntax_procedure, obj);
	obj->data.syntax_procedure.proc = proc;
	obj->data.syntax_procedure.id = id;
	return obj;
}

object* init_function(object* obj, object* environment, object* parameters, object* body) {
	init_object(location_stack, type_function, obj);
	obj->data.function.environment = environment;
	obj->data.function.parameters = parameters;
	obj->data.function.body = body;
	return obj;
}

object* init_call(object* obj, object* function, object* arguments, object* continuation) {
	init_object(location_stack, type_call, obj);
	obj->data.call.function = function;
	obj->data.call.arguments = arguments;
	obj->data.call.continuation = continuation;
	return obj;
}

object* init_cont_base(object* obj, object* call, cont_type t) {
	init_object(location_stack, type_continuation, obj);
	obj->data.continuation.conttype = t;
	obj->data.continuation.call = call;
	return obj;
}

object* init_cont(object* obj, object* call) {
	init_cont_base(obj, call, cont_normal);
	return obj;
}

object* init_discarding_cont(object* obj, object* call) {
	init_cont_base(obj, call, cont_discarding);
	return obj;
}

object* init_catching_cont(object* obj, object* call) {
	init_cont_base(obj, call, cont_catching);
	return obj;
}

object* init_syntax_object(object* obj, object* syntax, object* origin) {
	init_object(location_stack, type_syntax_object, obj);
	obj->data.syntax_object.syntax = syntax;
	obj->data.syntax_object.origin = origin;
	return obj;
}

object* init_internal_position(object* obj, int x, int y) {
	init_object(location_stack, type_internal_position, obj);
	obj->data.internal_position.x = x;
	obj->data.internal_position.y = y;
	return obj;
}

object* init_internal_error(object* obj, object* trace, object* message) {
	init_object(location_stack, type_internal_error, obj);
	obj->data.internal_error.trace = trace;
	obj->data.internal_error.message = message;
	return obj;
}

object* init_delay(object* obj, object* value) {
	init_object(location_stack, type_delay, obj);
	obj->data.delay.value = value;
	obj->data.delay.evaluated = 0;
	return obj;
}

object* init_primitive_procedure(object* obj, primitive_proc* proc) {
	init_object(location_static, type_primitive_procedure, obj);
	obj->data.primitive_procedure.proc = proc;
	return obj;
}

object* init_primitive(primitive_proc* proc, object* obj) {
	init_primitive_procedure(obj, proc);
	return obj;
}

object* init_list_cell(object* obj, object* first, object* rest) {
	init_object(location_stack, type_list, obj);
	obj->data.list.first = first;
	obj->data.list.rest = rest;
	return obj;
}

object* init_list_1(object* ls, object* first) {
	init_list_cell(ls, first, empty_list());
	return ls;
}

object* init_list_2(object* ls, object* first, object* second) {
	init_list_cell(ls, first, ls+1);
	init_list_1(ls+1, second);
	return ls;
}

object* init_list_3(object* ls, object* first, object* second, object* third) {
	init_list_cell(ls, first, ls+1);
	init_list_2(ls+1, second, third);
	return ls;
}

object* init_list_4(object* ls, object* first, object* second, object* third, object* fourth) {
	init_list_cell(ls, first, ls+1);
	init_list_3(ls+1, second, third, fourth);
	return ls;
}

object* init_list_5(object* ls, object* first, object* second, object* third, object* fourth, object* fifth) {
	init_list_cell(ls, first, ls+1);
	init_list_4(ls+1, second, third, fourth, fifth);
	return ls;
}

object* init_trace_list(object* cell, object* a, object* trace) {
	if (is_false(trace) || !is_syntax_object(a)) {
		return trace;
	}
	else {
		object* last = trace;
		
		while (!is_empty_list(trace)) {
			if (list_first(trace) == a) {
				return trace;
			}
			trace = list_rest(trace);
		}
		return init_list_cell(cell, a, last);
	}
}
