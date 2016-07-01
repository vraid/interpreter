#include "base-util.h"
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "delist.h"
#include "call.h"

object* quote_object(object* args, object* cont) {
	object* value;
	delist_1(args, &value);
	
	object ls[2];
	init_list_2(ls, quote_symbol(), value);
	
	return call_cont(cont, ls);
}

object* identity(object* args, object* cont) {
	object* obj;
	delist_1(args, &obj);
	
	return call_cont(cont, obj);
}

void init_base_util_procedures(void) {
	init_primitive_procedure(&quote_object_proc, &quote_object);
	init_primitive_procedure(&identity_proc, &identity);
}
