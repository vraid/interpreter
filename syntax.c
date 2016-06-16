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
		object message;
		object e;
		init_string(&message, "wrong number of parts after quote; should be 1");
		init_internal_error(&e, &message);
		return call_cont(cont, &e);
	}
}

object* check_syntax(object* args, object* cont) {
	object* syntax;
	delist_1(args, &syntax);

	object message;
	object e;
	
	if (is_empty_list(syntax)) {
		init_string(&message, "expression cannot be empty list");
		init_internal_error(&e, &message);
		return call_cont(cont, &e);
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
			init_string(&message, "unknown syntax");
			init_internal_error(&e, &message);
			return call_cont(cont, &e);
		}
	}
}

void init_syntax_procedures(void) {
	/*
	add_symbol_syntax(quote_symbol(), quote_syntax_proc);
	add_symbol_syntax(define_symbol(), define_syntax_proc);
	add_symbol_syntax(lambda_symbol(), lambda_syntax_proc);
	add_symbol_syntax(curry_symbol(), curry_syntax_proc);
	add_symbol_syntax(apply_symbol(), apply_syntax_proc);
	add_symbol_syntax(if_symbol(), if_syntax_proc);
	add_symbol_syntax(list_symbol(), list_syntax_proc);
	add_symbol_syntax(map_symbol(), map_syntax_proc);
	add_symbol_syntax(fold_symbol(), fold_syntax_proc);
	add_symbol_syntax(filter_symbol(), filter_syntax_proc);
	*/
}
