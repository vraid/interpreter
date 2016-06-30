#pragma once

#include "data-structures.h"

typedef enum {
	syntax_define,
	syntax_quote,
	syntax_delay,
	syntax_force,
	syntax_let,
	syntax_letrec,
	syntax_rec,
	syntax_lambda,
	syntax_curry,
	syntax_apply,
	syntax_if,
	syntax_and,
	syntax_or,
	syntax_struct,
	syntax_list,
	syntax_stream,
	syntax_vector,
	syntax_map,
	syntax_fold,
	syntax_filter,
	syntax_count} static_syntax;

object syntax_procedure[syntax_count];

object eval_force_proc;

void init_base_syntax_procedures(void);
