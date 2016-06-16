#include "base-util.h"
#include "data-structures.h"
#include "object-init.h"
#include "delist.h"
#include "call.h"

object _quote_object_proc;

object* quote_object_proc(void) {
	return &_quote_object_proc;
}

object* quote_object(object* args, object* cont) {
	object* value;
	delist_1(args, &value);
	
	object q;
	init_quote(&q, value);
	
	return call_cont(cont, &q);
}

void init_base_util_procedures(void) {
	init_primitive_procedure(quote_object_proc(), &quote_object);
}
