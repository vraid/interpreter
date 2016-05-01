#include "eval.h"
#include "data-structures.h"
#include "object-init.h"
#include "call.h"
#include "delist.h"

object eval_atom_proc;
object eval_function_proc;
object eval_call_proc;
object eval_proc;

object* eval_atom(object* args, object* cont) {
	return call_cont(cont, args);
}

object* eval_function(object* args, object* cont) {
	return call_cont(cont, args);
}

object* eval_call(object* args, object* cont) {
	return call_cont(cont, args);
}

object* eval(object* args, object* cont) {
	object* obj;
	delist_1(args, &obj);
	
	object* proc;

	object call;
	object ls[1];
	init_list_1(ls, obj);
	
	switch (obj->type) {
		case type_call:
			proc = &eval_call_proc;
			break;
		default:
			proc = &eval_atom_proc;
			break;
	}
	init_call(&call, proc, ls, cont);
	
	return perform_call(&call);
}

void init_eval_procedures(void) {
	init_primitive_procedure(&eval_atom_proc, &eval_atom);
	init_primitive_procedure(&eval_function_proc, &eval_function);
	init_primitive_procedure(&eval_call_proc, &eval_call);
	init_primitive_procedure(&eval_proc, &eval);
}
