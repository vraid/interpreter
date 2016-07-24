#include <stdlib.h>
#include <string.h>

#include "global-variables.h"
#include "data-structures.h"
#include "symbols.h"
#include "object-init.h"

char nullchar;
object _false;
object _true;
object _no_object;
object _no_symbol;
object _no_binding;
object _placeholder_value;
object _empty_list;
object _empty_stream;
object _empty_vector;
object _end_vector_iterator;
object _empty_string;
object _empty_environment;
object _empty_struct_definition;
object* _quote_symbol;
object _dash_string;
object _question_mark_string;
object _zero;
object _one;
object _negative_one;
object _bignum_zero_list;
object _bignum_zero;

object _end_cont;
object end_proc;
object end_call;

object* end_cont(void) {
	return &_end_cont;
}

object* end(object* args, object* cont) {
	printf("end reached\n");
	return no_object();
}

void init_boolean(object* obj, char value) {
	init_object(location_static, type_boolean, obj);
	obj->data.boolean.value = value;
}

void init_global_variables(void) {
	init_boolean(false(), 0);
	init_boolean(true(), 1);
	
	init_object(location_static, type_none, no_object());
	init_object(location_static, type_symbol, no_symbol());
	_no_symbol.data.symbol.name = empty_string();
	
	init_object(location_static, type_binding, no_binding());
	_no_binding.data.binding.name = no_symbol();
	_no_binding.data.binding.value = no_object();
	
	init_object(location_static, type_none, placeholder_value());
	
	init_object(location_static, type_list, empty_list());
	_empty_list.data.list.type = shapeless;
	_empty_list.data.list.first = no_object();
	_empty_list.data.list.rest = empty_list();

	init_object(location_static, type_stream, empty_stream());
	_empty_stream.data.stream.first = no_object();
	_empty_stream.data.stream.rest = no_object();

	init_object(location_static, type_vector, &_empty_vector);
	_empty_vector.data.vector.length = 0;
	_empty_vector.data.vector.data = 0;
	init_object(location_static, type_vector_iterator, end_vector_iterator());
	_end_vector_iterator.data.vector_iterator.n = 0;
	_end_vector_iterator.data.vector_iterator.vector = &_empty_vector;
	
	nullchar = 0;
	init_object(location_static, type_string, empty_string());
	_empty_string.data.string.value = &nullchar;
	_empty_string.data.string.length = 0;
	
	init_object(location_static, type_environment, &_empty_environment);
	_empty_environment.data.environment.bindings = empty_list();
	
	init_object(location_static, type_struct_definition, &_empty_struct_definition);
	_empty_struct_definition.data.struct_definition.name = no_symbol();
	_empty_struct_definition.data.struct_definition.fields = empty_list();
	_empty_struct_definition.data.struct_definition.parent = no_object();
	
	_quote_symbol = make_static_symbol("quote");
	
	init_string(dash_string(), "-");
	_dash_string.location = location_static;
	init_string(question_mark_string(), "?");
	_question_mark_string.location = location_static;
	
	init_object(location_static, type_fixnum, &_zero);
	_zero.data.fixnum.value = 0;
	init_object(location_static, type_fixnum, &_one);
	_one.data.fixnum.value = 1;
	init_object(location_static, type_fixnum, &_negative_one);
	_negative_one.data.fixnum.value = -1;
	
	init_object(location_static, type_list, &_bignum_zero_list);
	_bignum_zero_list.data.list.first = zero();
	_bignum_zero_list.data.list.rest = empty_list();

	init_object(location_static, type_bignum, &_bignum_zero);
	_bignum_zero.data.bignum.sign = 1;
	_bignum_zero.data.bignum.digits = &_bignum_zero_list;
	
	init_primitive_procedure(&end_proc, &end);
	init_call(&end_call, &end_proc, empty_list(), end_cont());
	init_cont(end_cont(), &end_call);
}

object* true(void) {
	return &_true;
}
object* false(void) {
	return &_false;
}
object* boolean(char b) {
	return b ? true() : false();
}
char is_empty_list(object* obj) {
	return obj == empty_list();
}
char is_empty_stream(object* obj) {
	return obj == empty_stream();
}
char is_end_vector_iterator(object* obj) {
	return obj == end_vector_iterator();
}
char is_empty_sequence(object* obj) {
	switch(obj->type) {
		case type_list: return is_empty_list(obj);
		case type_stream: return is_empty_stream(obj);
		case type_vector_iterator: return is_end_vector_iterator(obj);
		default: return 0;
	}
}
char is_false(object* obj) {
	return obj == false();
}
char is_true(object* obj) {
	return obj == true();
}
char is_no_object(object* obj) {
	return obj == no_object();
}
char is_no_binding(object* obj) {
	return obj == no_binding();
}
char is_placeholder_value(object* obj) {
	return obj == placeholder_value();
}
char is_empty_struct_definition(object* obj) {
	return obj == empty_struct_definition();
}
char is_zero(object* obj) {
	return obj == zero();
}
object* empty_list(void) {
	return &_empty_list;
}
object* empty_stream(void) {
	return &_empty_stream;
}
object* end_vector_iterator(void) {
	return &_end_vector_iterator;
}
object* empty_string(void) {
	return &_empty_string;
}
object* no_object(void) {
	return &_no_object;
}
object* no_symbol(void) {
	return &_no_symbol;
}
object* no_binding(void) {
	return &_no_binding;
}
object* placeholder_value(void) {
	return &_placeholder_value;
}
object* empty_environment(void) {
	return &_empty_environment;
}
object* empty_struct_definition(void) {
	return &_empty_struct_definition;
}
object* quote_symbol(void) {
	return _quote_symbol;
}
object* dash_string(void) {
	return &_dash_string;
}
object* question_mark_string(void) {
	return &_question_mark_string;
}
object* zero(void) {
	return &_zero;
}
object* one(void) {
	return &_one;
}
object* negative_one(void) {
	return &_negative_one;
}
object* sign_object(int sign) {
	switch (sign) {
		case 1: return one();
		case -1: return negative_one();
		default: {
			fprintf(stderr, "invalid sign: %i\n", sign);
			return no_object();
		}
	}
}

object* bignum_zero_list(void) {
	return &_bignum_zero_list;
}

object* bignum_zero(void) {
	return &_bignum_zero;
}
