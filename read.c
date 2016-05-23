#include "read.h"
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "delist.h"
#include "call.h"
#include "symbols.h"

object add_to_list_proc;
object read_list_value_proc;
object read_list_proc;
object start_list_proc;
object finish_list_proc;
object read_list_start_proc;

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

int peek(FILE* in) {
	int c = getc(in);
	ungetc(c, in);
	return c;
}

void consume_whitespace(FILE* in) {
	while (is_whitespace(peek(in))) {
		getc(in);	
	}
}

char buffer[1000];

object* read_identifier(FILE* in) {
	int i = 0;
	int c;
	while (!is_delimiter(c = peek(in))) {
		buffer[i] = c;
		getc(in);
		i++;
	}
	buffer[i] = 0;
	return symbol(buffer);
}

object* read_value(object* args, object* cont) {
	object* input_port;
	delist_1(args, &input_port);
	object value;
	
	FILE* in = file_port_file(input_port);
	consume_whitespace(in);
	
	if (is_list_start_delimiter(peek(in))) {
		getc(in);
		object call;
		init_call(&call, &read_list_start_proc, args, cont);
		return perform_call(&call);
	}
	else {
	}
	
	// read switch
	// symbol
	// number
	// list
	// boolean
	// string
	
	int c;
	getc(in);
	
	return call_cont(cont, no_object());
}

object* add_to_list(object* args, object* cont) {
	object* value;
	object* last;
	object* input;
	delist_3(args, &value, &last, &input);
	
	object next;
	init_list_cell(&next, value, empty_list());
	last->data.list.rest = &next;
	
	return call_cont(cont, &next);
}

object* read_list_value(object* args, object* cont) {
	object next_call;
	init_call(&next_call, &add_to_list_proc, args, cont);
	object next_cont;
	init_cont(&next_cont, &next_call);
	object call;
	init_call(&call, &read_value_proc, args, &next_cont);
	
	return perform_call(&call);
}

object* read_list(object* args, object* cont) {
	object* last;
	object* input;
	delist_2(args, &last, &input);
	
	FILE* in = file_port_file(input);
	consume_whitespace(in);
	if (is_list_end_delimiter(peek(in))) {
		getc(in);
		return call_cont(cont, no_object());
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
	init_call(&finish_call, &finish_list_proc, ls, cont);
	
	object finish_cont;
	init_discarding_cont(&finish_cont, &finish_call);
	
	object ls2[3];
	init_list_2(ls2, &first, input);
	
	// keep on building the list
	object call;
	init_call(&call, &read_list_proc, ls2, &finish_cont);
	
	return perform_call(&call);
}

object* finish_list(object* args, object* cont) {
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
		getc(in);
		return call_cont(cont, empty_list());
	}
	else {
		object call;
		init_call(&call, &start_list_proc, args, cont);
		
		return perform_call(&call);
	}
}

void init_read_procedures(void) {
	init_primitive_procedure(&read_value_proc, &read_value);
	init_primitive_procedure(&add_to_list_proc, &add_to_list);
	init_primitive_procedure(&read_list_value_proc, &read_list_value);
	init_primitive_procedure(&read_list_proc, &read_list);
	init_primitive_procedure(&start_list_proc, &start_list);
	init_primitive_procedure(&finish_list_proc, &finish_list);
	init_primitive_procedure(&read_list_start_proc, &read_list_start);
}
