#include "read.h"

#include <stdlib.h>
#include <string.h>
#include "read-state.h"
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "base-util.h"
#include "util.h"
#include "memory-handling.h"
#include "delist.h"
#include "call.h"
#include "symbols.h"
#include "integers.h"
#include "fractions.h"

object read_add_to_list_proc;
object read_list_value_proc;
object read_list_proc;
object start_list_proc;
object read_finish_list_proc;
object read_list_start_proc;

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

char is_whitespace(int c) {
	return c == ' ' || c == '\n' || c == ',';
}

char is_delimiter(int c) {
	return is_whitespace(c) || c == EOF ||
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

int peek(FILE* in) {
	int c = get_input(in);
	unget_input(in);
	return c;
}

void consume_whitespace(FILE* in) {
	while (is_whitespace(peek(in))) {
		get_input(in);	
	}
}

#define input_buffer_size 1024
char input_buffer[input_buffer_size];

char keep_reading(char string, int c, int escapes) {
	if (string) {
		return (escapes & 1) || !(is_quotation_mark(c));
	}
	else {
		return !is_delimiter(c);
	}
}

char* get_string(char string, FILE* in) {
	if (string) get_input(in);
	int i = 0;
	int c = get_input(in);
	int escapes = 0;
	while (keep_reading(string, c, escapes)) {
		input_buffer[i] = c;
		i++;
		if (i >= input_buffer_size) {
			fprintf(stderr, "identifier too long, max %i characters\n", input_buffer_size);
		}
		escapes = is_escape_char(c) ? escapes + 1 : 0;
		c = get_input(in);
	}
	if (!string) unget_input(in);
	
	input_buffer[i] = 0;
	return input_buffer;
}

object* string(char* cs, object* cont) {
	char* str = alloca(sizeof(char) * (1 + strlen(cs)));
	strcpy(str, cs);
	object s;
	init_string(&s, str);
	return call_cont(cont, &s);
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
		object next_index;
		init_fixnum(&next_index, fixnum_value(index) + 1);
		
		object digit;
		init_fixnum(&digit, string_value(string)[i] - '0');
		object ls[1];
		init_list_1(ls, &digit);
		
		object read_args[2];
		init_list_2(read_args, &next_index, string);
		object read_call;
		init_call(&read_call, &read_integer_proc, read_args, cont);
		object read_cont;
		init_cont(&read_cont, &read_call);
		
		object add_args[1];
		init_list_1(add_args, ls);
		object add_call;
		init_call(&add_call, &integer_add_signless_proc, add_args, &read_cont);
		object add_cont;
		init_cont(&add_cont, &add_call);
		
		object multiply_args[2];
		init_list_2(multiply_args, num, integer_ten_list());
		object multiply_call;
		init_call(&multiply_call, &integer_multiply_digits_proc, multiply_args, &add_cont);
		
		return perform_call(&multiply_call);
	}
}

object* read_number(object* args, object* cont) {
	object* string;
	delist_1(args, &string);
	
	if (is_valid_number(string)) {
		int sign = string_value(string)[0] == '-' ? -1 : 1;
		
		int i = (sign == 1) ? 0 : 1;
		object index;
		init_fixnum(&index, i);
		
		object fraction_args[1];
		init_list_1(fraction_args, integer_one());
		object fraction_call;
		init_call(&fraction_call, &make_fraction_proc, fraction_args, cont);
		object fraction_cont;
		init_cont(&fraction_cont, &fraction_call);
		
		object make_args[1];
		init_list_1(make_args, sign_object(sign));
		object make_call;
		init_call(&make_call, &make_integer_proc, make_args, &fraction_cont);
		object make_cont;
		init_cont(&make_cont, &make_call);
		
		object read_args[3];
		init_list_3(read_args, integer_zero_list(), &index, string);
		object read_call;
		init_call(&read_call, &read_integer_proc, read_args, &make_cont);
		
		return perform_call(&read_call);
	}
	else {
		object str;
		init_string(&str, "invalid number");
		object ls[2];
		init_list_2(ls, &str, string);
		return throw_error(cont, ls);
	}
}

object* read_hashed(object* args, object* cont) {
	object* string;
	delist_1(args, &string);
	
	if (string_length(string) == 2) {
		char* str = string_value(string);
		switch (str[1]) {
			case 't' : return call_cont(cont, true());
			case 'f' : return call_cont(cont, false());
		}
	}
	object str;
	init_string(&str, "invalid value");
	object ls[2];
	init_list_2(ls, &str, string);
	return throw_error(cont, ls);
}

object make_syntax_proc;

object* make_syntax(object* args, object* cont) {
	object* stx;
	object* pos;
	delist_2(args, &stx, &pos);
	
	object obj;
	init_syntax_object(&obj, stx, pos);
	
	return call_cont(cont, &obj);
}

object read_value_proc;

object* read_value(object* args, object* cont) {
	object* input_port;
	delist_1(args, &input_port);
	
	FILE* in = file_port_file(input_port);
	consume_whitespace(in);
	
	char c = peek(in);
	
	object current_pos;
	init_internal_position(&current_pos, current_read_state().x, current_read_state().y);
	
	object syntax_args[1];
	init_list_1(syntax_args, &current_pos);
	object syntax_call;
	init_call(&syntax_call, &make_syntax_proc, syntax_args, cont);
	object syntax_cont;
	init_cont(&syntax_cont, &syntax_call);
	
	if (is_list_end_delimiter(c)) {
		get_input(in);
		return throw_error_string(cont, "unexpected parenthesis");
	}
	else if (is_list_start_delimiter(c)) {
		get_input(in);
		object call;
		init_call(&call, &read_list_start_proc, args, &syntax_cont);
		return perform_call(&call);
	}
	else if (is_quote_char(c)) {
		get_input(in);
		object quote_call;
		init_call(&quote_call, &quote_object_proc, empty_list(), &syntax_cont);
		object quote_cont;
		init_cont(&quote_cont, &quote_call);
		object read_call;
		init_call(&read_call, &read_value_proc, args, &quote_cont);
		return perform_call(&read_call);
	}
	else {
		char q = is_quotation_mark(c);
		char* str = get_string(q, in);
		
		object call;
		object* primitive;
		if (q) {
			primitive = &read_string_proc;
		}
		else if (is_hash_char(c)) {
			primitive = &read_hashed_proc;
		}
		else if (is_digit(c) || ((c == '-') && (strlen(str) > 1))) {
			primitive = &read_number_proc;
		}
		else {
			primitive = &read_nonstring_proc;
		}
		init_call(&call, primitive, empty_list(), &syntax_cont);
		
		object next_cont;
		init_cont(&next_cont, &call);
		
		return string(str, &next_cont);
	}
}

object* read_add_to_list(object* args, object* cont) {
	object* value;
	object* last;
	object* input;
	delist_3(args, &value, &last, &input);
	
	object next;
	init_list_cell(&next, value, empty_list());
	last->data.list.rest = &next;
	add_mutation(last, &next);
	
	object ls[2];
	init_list_2(ls, &next, input);
	
	object call;
	init_call(&call, &read_list_proc, ls, cont);
	
	return perform_call(&call);
}

object* read_list_value(object* args, object* cont) {
	object* last;
	object* input;
	delist_2(args, &last, &input);
	
	object next_call;
	init_call(&next_call, &read_add_to_list_proc, args, cont);
	object next_cont;
	init_cont(&next_cont, &next_call);

	object ls[1];
	init_list_1(ls, input);
	
	object call;
	init_call(&call, &read_value_proc, ls, &next_cont);
	
	return perform_call(&call);
}

object* read_list(object* args, object* cont) {
	object* last;
	object* input;
	delist_2(args, &last, &input);
	
	FILE* in = file_port_file(input);
	consume_whitespace(in);
	if (is_list_end_delimiter(peek(in))) {
		get_input(in);
		return call_discarding_cont(cont);
	}
	else {
		object call;
		init_call(&call, &read_list_value_proc, args, cont);
		return perform_call(&call);
	}
}

object* start_list(object* args, object* cont) {
	object* value;
	object* input;
	delist_2(args, &value, &input);
	
	object first;
	init_list_cell(&first, value, empty_list());
	
	object ls[1];
	init_list_1(ls, &first);
	
	// after the list is finished, pass it on to the cont
	object finish_call;
	init_call(&finish_call, &read_finish_list_proc, ls, cont);
	
	object finish_cont;
	init_discarding_cont(&finish_cont, &finish_call);
	
	object ls2[2];
	init_list_2(ls2, &first, input);
	
	// keep on building the list
	object call;
	init_call(&call, &read_list_proc, ls2, &finish_cont);
	
	return perform_call(&call);
}

object* read_finish_list(object* args, object* cont) {
	object* first;
	delist_1(args, &first);
	
	return call_cont(cont, first);
}

object* read_list_start(object* args, object* cont) {
	object* input;
	delist_1(args, &input);
	
	FILE* in = file_port_file(input);
	consume_whitespace(in);
	
	if (is_list_end_delimiter(peek(in))) {
		get_input(in);
		return call_cont(cont, empty_list());
	}
	else {
		object next_call;
		init_call(&next_call, &start_list_proc, args, cont);
		
		object next_cont;
		init_cont(&next_cont, &next_call);
		
		object call;
		init_call(&call, &read_value_proc, args, &next_cont);
		
		return perform_call(&call);
	}
}

object* read_entry(object* args, object* cont) {
	object* input_port;
	delist_1(args, &input_port);
	
	consume_whitespace(file_port_file(input_port));
	reset_read_state();
	
	return read_value(args, cont);
}

void init_read_procedures(void) {
	init_primitive(&read_entry, &read_entry_proc);
	init_primitive(&read_value, &read_value_proc);
	init_primitive(&make_syntax, &make_syntax_proc);
	init_primitive(&read_add_to_list, &read_add_to_list_proc);
	init_primitive(&read_list_value, &read_list_value_proc);
	init_primitive(&read_list, &read_list_proc);
	init_primitive(&start_list, &start_list_proc);
	init_primitive(&read_finish_list, &read_finish_list_proc);
	init_primitive(&read_list_start, &read_list_start_proc);
	init_primitive(&read_string, &read_string_proc);
	init_primitive(&read_nonstring, &read_nonstring_proc);
	init_primitive(&read_number, &read_number_proc);
	init_primitive(&read_integer, &read_integer_proc);
	init_primitive(&read_hashed, &read_hashed_proc);
}
