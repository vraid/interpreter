#include "syntax.h"

#include <stdlib.h>
#include "call.h"
#include "object-init.h"
#include "delist.h"
#include "global-variables.h"

#define symbol_syntax_max 1024

typedef struct {
	object* symbol;
	object* syntax;
} t_symbol_syntax;

t_symbol_syntax symbol_syntax[symbol_syntax_max];
int symbol_syntax_count = 0;

void add_symbol_syntax(object* symbol, object* syntax) {
	if (symbol_syntax_count >= symbol_syntax_max) {
		fprintf(stderr, "max symbol syntax reached\n");
		exit(0);
	}
	symbol_syntax[symbol_syntax_count].symbol = symbol;
	symbol_syntax[symbol_syntax_count].syntax = syntax;
	symbol_syntax_count++;
}

object* quote_syntax_proc;
object* define_syntax_proc;
object* lambda_syntax_proc;
object* curry_syntax_proc;
object* apply_syntax_proc;
object* if_syntax_proc;
object* list_syntax_proc;
object* map_syntax_proc;
object* fold_syntax_proc;
object* filter_syntax_proc;

object* check_quote_syntax(object* args, object* cont) {
	object* syntax;
	delist_1(args, &syntax);
	
	int length = list_length(syntax);
	if (length == 2) {
		return call_cont(cont, syntax);
	}
	else {
		return throw_error(cont, "wrong number of parts after quote; should be 1");
	}
}

object* check_syntax(object* args, object* cont) {
	object* syntax;
	delist_1(args, &syntax);
	
	if (is_empty_list(syntax)) {
		return throw_error(cont, "expression cannot be empty list");
	}
	else {
		object* first = list_first(syntax);
		if (is_list(first)) {
			return call_cont(cont, syntax);
		}
		else if (is_symbol(first)) {
			int i;
			for (i = 0; i < symbol_syntax_count; i++) {
				if (first == symbol_syntax[i].symbol) {
					object call;
					init_call(&call, symbol_syntax[i].syntax, args, cont);
					return perform_call(&call);
				}
			}
			return call_cont(cont, syntax);
		}
		else {
			return throw_error(cont, "unknown syntax");
		}
	}
}

void init_syntax_procedures(void) {
}
