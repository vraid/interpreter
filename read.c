#include "read.h"

#include <stdlib.h>
#include <string.h>
#include "ports.h"
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "base-util.h"
#include "list-util.h"
#include "string-util.h"
#include "util.h"
#include "memory-handling.h"
#include "delist.h"
#include "call.h"
#include "symbols.h"
#include "integers.h"
#include "fractions.h"
#include "syntax-base.h"

object read_list_proc;
object read_value_proc;
object read_string_proc;
object read_nonstring_proc;
object read_number_proc;
object read_hashed_proc;

char in_bracket_list(char* ls, int c) {
	int i;
	for (i = 0; i < bracket_type_count; i++) {
		if (c == ls[i]) {
			return 1;
		}
	}
	return 0;
}

char is_list_start_delimiter(int c) {
	return in_bracket_list(list_start_delimiter, c);
}

char is_list_end_delimiter(int c) {
	return in_bracket_list(list_end_delimiter, c);
}

char is_list_delimiter(int c) {
	return is_list_start_delimiter(c) || is_list_end_delimiter(c);
}

bracket_type list_delimiter_type(int c) {
	switch (c) {
		case '(':
		case ')': return round;
		case '[':
		case ']': return square;
		case '{':
		case '}': return curly;
		default: return bracket_type_count;
	}
}

char is_eof(int c) {
	return c == EOF;
}

char is_whitespace(int c) {
	return c == ' ' || c == '\n' || c == ',';
}

char is_delimiter(int c) {
	return is_whitespace(c) ||
		is_list_delimiter(c) ||
		c == '"';
}

char is_escape_char(int c) {
	return c == '\\';
}

char is_quotation_mark(int c) {
	return c == '"';
}

char is_quote_char(int c) {
	return c == '\'';
}

char is_hash_char(int c) {
	return c == '#';
}

char is_digit(int c) {
	return (c >= '0') && (c <= '9');
}

char is_valid_number(object* string) {
	int i;
	char* str = string_value(string);
	for (i = 0; i < string_length(string); i++) {
		if (!(is_digit(str[i]) || ((i == 0) && (str[0] == '-') && (string_length(string) > 1)))) return 0;
	}
	return 1;
}

void consume_whitespace(object* obj) {
	while (is_whitespace(peek(obj))) {
		get_input(obj);	
	}
}

#define input_buffer_size 1024
char input_buffer[input_buffer_size];

char keep_reading(char string, int c, int escapes) {
	if (is_eof(c)) {
		return 0;
	}
	else if (string) {
		return (escapes & 1) || !(is_quotation_mark(c));
	}
	else {
		return !is_delimiter(c);
	}
}

char* get_string(char string, object* obj) {
	if (string) get_input(obj);
	int i = 0;
	int c = get_input(obj);
	int escapes = 0;
	while (keep_reading(string, c, escapes)) {
		input_buffer[i] = c;
		i++;
		if (i >= input_buffer_size) {
			fprintf(stderr, "identifier too long, max %i characters\n", input_buffer_size);
		}
		escapes = is_escape_char(c) ? escapes + 1 : 0;
		c = get_input(obj);
	}
	if (!string) unget_input(obj);
	
	input_buffer[i] = 0;
	return input_buffer;
}

object* read_true(object* args, object* cont) {
	suppress_warning(args);
	return call_cont(cont, true());
}

object* read_false(object* args, object* cont) {
	suppress_warning(args);
	return call_cont(cont, false());
}

object desyntax_included_proc;

object* desyntax_included(object* args, object* cont) {
	object* stx;
	delist_1(args, &stx);
	
	return call_cont(cont, desyntax(stx));
}

object assert_read_empty_proc;

object* assert_read_empty(object* args, object* cont) {
	object* result;
	object* filename;
	object* input_port;
	delist_3(args, &result, &filename, &input_port);
	
	consume_whitespace(input_port);
	
	char c = peek(input_port);
	if (!is_eof(c)) {
		return throw_error(cont, alloc_string_append_2(alloc_string("expected single expression in file: "), filename));
	}
	
	return call_cont(cont, result);
}

object* read_include(object* args, object* cont) {
	object* ls;
	object* read_table;
	delist_2(args, &ls, &read_table);
	object* filename;
	delist_desyntax_1(desyntax(ls), &filename);
	
	FILE* f = fopen(string_value(filename), "r");
	if (f == NULL) {
		return throw_error(cont, alloc_string_append_2(alloc_string("file not found: "), filename));
	}
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);
	
	char* mem = malloc(size+1);
	fread(mem, size, 1, f);
	fclose(f);
	
	alloc_malloc_reference(size+1, mem);
	object* str = alloc_string(mem);
	
	object* input_port = alloc_string_port(str);
	
	object* desyntax_call = alloc_call(&desyntax_included_proc, empty_list(), cont);
	object* empty_call = alloc_call(&assert_read_empty_proc, alloc_list_2(filename, input_port), alloc_cont(desyntax_call));
	
	object* read_call = alloc_call(&read_entry_proc, alloc_list_2(input_port, read_table), alloc_cont(empty_call));
	
	return perform_call(read_call);
}

object* read_enter_scope(object* args, object* cont) {
	object* ls;
	object* read_table;
	delist_desyntax_2(args, &ls, &read_table);
	
	object* key = no_symbol();
	if (!is_empty_list(ls)) {
		delist_desyntax_1(ls, &key);
		if (!is_symbol(key)) {
			return throw_error_string(cont, "enter-scope may only take symbol");
		}
	}
	
	object* stx = alloc_list_3(syntax_procedure_obj(syntax_enter_scope), key, read_table);
	
	return call_cont(cont, stx);
}

object* read_rewind_scope(object* args, object* cont) {
	object* ls;
	object* read_table;
	delist_desyntax_2(args, &ls, &read_table);
	
	object* key = no_symbol();
	if (!is_empty_list(ls)) {
		delist_desyntax_1(ls, &key);
		if (!is_symbol(key)) {
			return throw_error_string(cont, "rewind-scope may only take symbol");
		}
	}
	
	object* stx = alloc_list_2(syntax_procedure_obj(syntax_rewind_scope), key);
	
	return call_cont(cont, stx);
}

object* string(char* cs, object* cont) {
	char* str = alloc_copy_str(cs);
	object* s = alloc_string(str);
	return call_cont(cont, s);
}

object* read_string(object* args, object* cont) {
	object* string;
	delist_1(args, &string);
	
	return call_cont(cont, string);
}

object* read_nonstring(object* args, object* cont) {
	object* string;
	delist_1(args, &string);
	
	return symbol(string_value(string), cont);
}

object read_integer_proc;

object* read_integer(object* args, object* cont) {
	object* num;
	object* index;
	object* string;
	delist_3(args, &num, &index, &string);

	fixnum_type i = fixnum_value(index);
	if (i == string_length(string)) {
		return call_cont(cont, num);
	}
	else {
		object* next_index = alloc_fixnum(fixnum_value(index) + 1);
		
		object* digit = alloc_fixnum(string_value(string)[i] - '0');
		object* ls = alloc_list_1(digit);
		
		object* read_args = alloc_list_2(next_index, string);
		object* read_call = alloc_call(&read_integer_proc, read_args, cont);
		
		object* add_args = alloc_list_1(ls);
		object* add_call = alloc_call(&integer_add_signless_proc, add_args, alloc_cont(read_call));
		
		object* multiply_args = alloc_list_2(num, integer_ten_list());
		object* multiply_call = alloc_call(&integer_multiply_digits_proc, multiply_args, alloc_cont(add_call));
		
		return perform_call(multiply_call);
	}
}

object* read_number(object* args, object* cont) {
	object* string;
	delist_1(args, &string);
	
	if (is_valid_number(string)) {
		int sign = string_value(string)[0] == '-' ? -1 : 1;
		
		int i = (sign == 1) ? 0 : 1;
		object* index = alloc_fixnum(i);
		
		object* fraction_args = alloc_list_1(integer_one());
		object* fraction_call = alloc_call(&make_fraction_proc, fraction_args, cont);
		
		object* make_args = alloc_list_1(sign_object(sign));
		object* make_call = alloc_call(&make_integer_proc, make_args, alloc_cont(fraction_call));
		
		object* read_args = alloc_list_3(integer_zero_list(), index, string);
		object* read_call = alloc_call(&read_integer_proc, read_args, alloc_cont(make_call));
		
		return perform_call(read_call);
	}
	else {
		return throw_error(cont, alloc_string_append_2(alloc_string("invalid number: "), string));
	}
}

object read_atom_type_proc;

object* read_atom_type(object* args, object* cont) {
	object* proc;
	object* input_port;
	object* read_table;
	delist_3(args, &proc, &input_port, &read_table);
	
	object* call = alloc_call(proc, empty_list(), cont);
	return perform_call(call);
}

object assert_is_list_proc;

object* assert_is_list(object* args, object* cont) {
	object* value;
	delist_1(args, &value);
	
	if (!is_list(desyntax(value))) {
		object* str = alloc_string_append_2(alloc_string("expected list, received "), alloc_string(object_type_name(value)));
		return throw_error(cont, alloc_list_2(str, value));
	}
	else {
		return call_cont(cont, value);
	}
}

object read_list_type_proc;

object* read_list_type(object* args, object* cont) {
	object* proc;
	object* input_port;
	object* read_table;
	delist_3(args, &proc, &input_port, &read_table);
	
	object* next = alloc_call(proc, alloc_list_1(read_table), cont);
	object* assertion = alloc_call(&assert_is_list_proc, empty_list(), alloc_cont(next));
	object* call = alloc_call(&read_value_proc, list_rest(args), alloc_cont(assertion));
	return perform_call(call);
}

object* read_hashed(object* args, object* cont) {
	object* string;
	object* trailing_parenthesis;
	object* input_port;
	object* read_table;
	delist_4(args, &string, &trailing_parenthesis, &input_port, &read_table);
	
	char* sym = string_value(string)+1;
	object* table = read_table;
	
	while (!is_empty_list(table)) {
		object* a = list_first(table);
		object* id;
		object* entry;
		delist_2(a, &id, &entry);
		if (!strcmp(sym, string_value(id))) {
			read_type t = reader_entry_read_type(entry);
			object* proc;
			if (t == read_type_atom) {
				proc = &read_atom_type_proc;
			}
			else {
				proc = &read_list_type_proc;
				if (is_false(trailing_parenthesis)) {
					object* str = alloc_string("expected parenthesis after read syntax");
					object* ls = alloc_list_2(str, string);
					return throw_error(cont, ls);
				}
			}
			object* call = alloc_call(proc, alloc_list_3(reader_entry_proc(entry), input_port, read_table), cont);
			return perform_call(call);
		}
		table = list_rest(table);
	}
	object* str = alloc_string("unknown read syntax");
	object* ls = alloc_list_2(str, string);
	return throw_error(cont, ls);
}

object make_syntax_proc;

object* make_syntax(object* args, object* cont) {
	object* stx;
	object* pos;
	delist_2(args, &stx, &pos);
	
	object* obj = alloc_syntax_object(stx, pos);
	return call_cont(cont, obj);
}

object* read_value(object* args, object* cont) {
	object* input_port;
	object* read_table;
	delist_2(args, &input_port, &read_table);
	
	consume_whitespace(input_port);
	
	char c = peek(input_port);
	
	object* current_pos = alloc_internal_position(port_position(input_port));
	
	object* syntax_args = alloc_list_1(current_pos);
	object* syntax_call = alloc_call(&make_syntax_proc, syntax_args, cont);
	object* syntax_cont = alloc_cont(syntax_call);
	
	if (is_eof(c)) {
		return throw_error_string(cont, "unexpected end of file");
	}
	else if (is_list_end_delimiter(c)) {
		get_input(input_port);
		return throw_error_string(cont, "unexpected parenthesis");
	}
	else if (is_list_start_delimiter(c)) {
		get_input(input_port);
		object* call = alloc_call(&read_list_proc, alloc_list_cell(empty_list(), args), syntax_cont);
		return perform_call(call);
	}
	else if (is_quote_char(c)) {
		get_input(input_port);
		object* quote_call = alloc_call(&quote_object_proc, empty_list(), syntax_cont);
		object* read_call = alloc_call(&read_value_proc, args, alloc_cont(quote_call));
		return perform_call(read_call);
	}
	else {
		char q = is_quotation_mark(c);
		char* str = get_string(q, input_port);
		
		object* primitive;
		object* primitive_args = empty_list();
		if (q) {
			primitive = &read_string_proc;
		}
		else if (is_hash_char(c)) {
			primitive = &read_hashed_proc;
			primitive_args = alloc_list_3(boolean(peek(input_port) == '('), input_port, read_table);
		}
		else if (is_digit(c) || ((c == '-') && (strlen(str) > 1))) {
			primitive = &read_number_proc;
		}
		else {
			primitive = &read_nonstring_proc;
		}
		object* call = alloc_call(primitive, primitive_args, syntax_cont);
		
		return string(str, alloc_cont(call));
	}
}

object* read_list(object* args, object* cont) {
	object* last;
	object* input;
	object* read_table;
	delist_3(args, &last, &input, &read_table);
	
	consume_whitespace(input);
	if (is_list_end_delimiter(peek(input))) {
		get_input(input);
		object* call = alloc_call(&reverse_list_proc, alloc_list_1(last), cont);
		return perform_call(call);
	}
	else {
		object* next_call = alloc_call(&read_list_proc, list_rest(args), cont);
		object* link_call = alloc_call(&link_list_proc, alloc_list_1(last), alloc_cont(next_call));
		object* call = alloc_call(&read_value_proc, list_rest(args), alloc_cont(link_call));
		return perform_call(call);
	}
}

object* read_entry(object* args, object* cont) {
	object* input_port;
	object* read_table;
	delist_2(args, &input_port, &read_table);
	
	return read_value(args, cont);
}

void init_read_procedures(void) {
	init_primitive(&read_entry, &read_entry_proc);
	init_primitive(&read_value, &read_value_proc);
	init_primitive(&make_syntax, &make_syntax_proc);
	init_primitive(&read_list, &read_list_proc);
	init_primitive(&read_string, &read_string_proc);
	init_primitive(&read_nonstring, &read_nonstring_proc);
	init_primitive(&read_number, &read_number_proc);
	init_primitive(&read_integer, &read_integer_proc);
	init_primitive(&read_hashed, &read_hashed_proc);
	init_primitive(&read_atom_type, &read_atom_type_proc);
	init_primitive(&read_list_type, &read_list_type_proc);
	init_primitive(&assert_is_list, &assert_is_list_proc);
	init_primitive(&read_true, &read_true_proc);
	init_primitive(&read_false, &read_false_proc);
	init_primitive(&read_include, &read_include_proc);
	init_primitive(&read_enter_scope, &read_enter_scope_proc);
	init_primitive(&read_rewind_scope, &read_rewind_scope_proc);
	init_primitive(&desyntax_included, &desyntax_included_proc);
	init_primitive(&assert_read_empty, &assert_read_empty_proc);
}
