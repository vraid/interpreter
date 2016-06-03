#include <stdio.h>
#include "print.h"
#include "data-structures.h"
#include "global-variables.h"
#include "call.h"
#include "delist.h"
#include "object-init.h"

object* print_value(object* args, object* cont) {
	object* obj;
	delist_1(args, &obj);
	
	switch (obj->type) {
		case type_none:
			printf("undefined");
			break;
		case type_symbol:
			printf("%s", string_value(symbol_name(obj)));
			break;
		case type_boolean:
			if (is_false(obj)) {
				printf("#f");
			}
			else if (is_true(obj)) {
				printf("#t");
			}
			else {
				printf("error\n");
				fprintf(stderr, "erroneous boolean");
			}
			break;
		case type_number:
			printf("%ld", number_value(obj));
			break;
		case type_list:
			printf("list");
			break;
		case type_function:
			printf("function ");
			break;
		default:
			fprintf(stderr, "unknown type");
	}	
	printf("\n");
	
	return call_discarding_cont(cont);
}

void init_print_procedures(void) {
	init_primitive_procedure(&print_proc, &print_value);
}
