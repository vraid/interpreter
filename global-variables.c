#include <stdlib.h>
#include <string.h>

#include "global-variables.h"
#include "data-structures.h"
#include "symbols.h"
#include "object-init.h"
#include "base-util.h"

char nullchar;
object _nothing;
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
object* _define_symbol;
object* _quote_symbol;
object _dash_string;
object _question_mark_string;
object _zero;
object _one;
object _negative_one;
object _ten;
object _integer_zero_list;
object _integer_zero;
object _integer_one_list;
object _integer_one;
object _integer_ten_list;
object _integer_ten;

object _default_context;
object _module_context;
object _scope_context;
object _repl_context;

object _end_cont;
object end_proc;
object end_call;

object* end_cont(void) {
	return &_end_cont;
}

object* end(object* args, object* cont) {
	suppress_warning(args);
	suppress_warning(cont);
	printf("end reached\n");
	return no_object();
}

void init_boolean(object* obj, char value) {
	init_object(location_static, type_boolean, obj);
	obj->data.boolean.value = value;
}

void init_static_fixnum(object* obj, long value) {
	init_fixnum(obj, value);
	make_static(obj);
}

void init_eval_context_obj(object* obj, char* name, context_type type) {
	init_eval_context(obj, type);
	make_static(obj);
	context_names[type] = name;
}

void init_global_variables(void) {
	init_object(location_static, type_nothing, nothing());
	
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
	
	_define_symbol = make_static_symbol("define");
	_quote_symbol = make_static_symbol("quote");
	
	init_string(dash_string(), "-");
	make_static(&_dash_string);
	init_string(question_mark_string(), "?");
	make_static(&_question_mark_string);
	
	init_static_fixnum(&_zero, 0);
	init_static_fixnum(&_one, 1);
	init_static_fixnum(&_negative_one, -1);
	init_static_fixnum(&_ten, 10);
	
	init_list_1(&_integer_zero_list, zero());
	make_static(&_integer_zero_list);

	init_integer(&_integer_zero, 0, &_integer_zero_list);
	make_static(&_integer_zero);
	
	init_list_1(&_integer_one_list, one());
	make_static(&_integer_one_list);

	init_integer(&_integer_one, 1, &_integer_one_list);
	make_static(&_integer_one);
	
	init_list_1(&_integer_ten_list, &_ten);
	make_static(&_integer_ten_list);

	init_integer(&_integer_ten, 1, &_integer_ten_list);
	make_static(&_integer_ten);
	
	init_primitive(&end, &end_proc);
	make_static(&end_proc);
	init_call(&end_call, &end_proc, empty_list(), end_cont());
	make_static(&end_call);
	init_cont(end_cont(), &end_call);
	make_static(&_end_cont);
	
	context_type t;
	for (t = 0; t <= context_count; t++) {
		context_names[t] = "";
	}
	
	init_eval_context_obj(&_default_context, "default", context_value);
	init_eval_context_obj(&_module_context, "module", context_module);
	init_eval_context_obj(&_scope_context, "scope", context_value | context_scope);
	init_eval_context_obj(&_repl_context, "repl", context_value | context_repl);
}

object* nothing(void) {
	return &_nothing;
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
char is_no_symbol(object* obj) {
	return obj == no_symbol();
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
object* define_symbol(void) {
	return _define_symbol;
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
		case 0: return zero();
		case 1: return one();
		case -1: return negative_one();
		default: {
			fprintf(stderr, "invalid sign: %i\n", sign);
			return no_object();
		}
	}
}

object* ten(void) {
	return &_ten;
}

object* integer_zero_list(void) {
	return &_integer_zero_list;
}
object* integer_zero(void) {
	return &_integer_zero;
}
object* integer_one_list(void) {
	return &_integer_one_list;
}
object* integer_one(void) {
	return &_integer_one;
}
object* integer_ten_list(void) {
	return &_integer_ten_list;
}
object* integer_ten(void) {
	return &_integer_ten;
}

object* default_context(void) {
	return &_default_context;
}
object* module_context(void) {
	return &_module_context;
}
object* scope_context(void) {
	return &_scope_context;
}
object* repl_context(void) {
	return &_repl_context;
}
