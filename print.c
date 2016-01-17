#include <stdio.h>

#include "print.h"
#include "data-structures.h"
#include "global-variables.h"

void write(object* obj);

void write_list_cell(char first, object* obj) {
	if (is_empty_list(obj)) {
		printf("%c", list_end_delimiter[list_type(obj)]);
	}
	else {
		if (!first) {
			printf(" ");
		}
		write(list_first(obj));
		write_list_cell(0, list_rest(obj));
	}
}

void write_list(object* obj) {
	printf("%c", list_start_delimiter[list_type(obj)]);
	write_list_cell(1, obj);
}

void write(object* obj) {
	switch(obj->type) {
		case type_none:
			printf("undefined");
			break;
		case type_symbol:
			printf(symbol_name(obj));
			break;
		case type_boolean:
			if (is_false(obj)) {
				printf("#f");
			}
			else if (is_true(obj)) {
				printf("#t");
			}
			else {
				fprintf(stderr, "erroneous boolean");
			}
			break;
		case type_number:
			printf("%ld", number_value(obj));
			break;
		case type_list:
			if (is_quote_symbol(list_first(obj))) {
				printf("'");
				write(list_ref(1, obj));
			}
			else {
				write_list(obj);
			}
			break;
		case type_function:
			printf("function ");
			write_list(function_parameters(obj));
			break;
		default:
			fprintf(stderr, "unknown type");
	}
}