#include "function-eval.h"

#include <stdlib.h>
#include "data-structures.h"
#include "object-init.h"
#include "global-variables.h"
#include "environments.h"
#include "eval.h"
#include "call.h"
#include "delist.h"
#include "list-util.h"

object eval_primitive_procedure_call_proc;

object* eval_primitive_procedure_call(object* args, object* cont) {
	object* arguments;
	object* proc;
	object* environment;
	object* context;
	object* trace;
	delist_5(args, &arguments, &proc, &environment, &context, &trace);
	
	object* call = alloc_call(proc, arguments, cont);
	
	return perform_call(call);
}

object eval_primitive_procedure_proc;

object* eval_primitive_procedure(object* args, object* cont) {
	object* proc;
	object* arguments;
	object* environment;
	object* context;
	object* trace;
	delist_5(args, &proc, &arguments, &environment, &context, &trace);
	
	object* proc_ls = alloc_list_4(proc, environment, context, trace);
	object* proc_call = alloc_call(&eval_primitive_procedure_call_proc, proc_ls, cont);
	
	object* ls = alloc_list_4(arguments, environment, default_context(), trace);
	object* call = alloc_call(&eval_list_elements_proc, ls, alloc_cont(proc_call));
	
	return perform_call(call);
}

object* eval_function_call(object* args, object* cont) {
	object* arguments;
	object* function;
	object* context;
	object* trace;
	delist_4(args, &arguments, &function, &context, &trace);
	
	function = desyntax(function);
	
	object* eval_args = alloc_list_3(function_body(function), default_context(), trace);
	object* eval_call = alloc_call(&eval_with_environment_proc, eval_args, cont);
	
	object* bind_args = alloc_list_3(arguments, function_parameters(function), function_environment(function));
	object* bind_call = alloc_call(&bind_values_proc, bind_args, alloc_cont(eval_call));
	
	return perform_call(bind_call);
}

object eval_function_proc;

object* eval_function(object* args, object* cont) {
	object* function;
	object* arguments;
	object* environment;
	object* context;
	object* trace;
	delist_5(args, &function, &arguments, &environment, &context, &trace);
	
	object* call_args = alloc_list_3(function, context, trace);
	object* call = alloc_call(&eval_function_call_proc, call_args, cont);
	
	object* eval_args = alloc_list_4(arguments, environment, default_context(), trace);
	object* eval_call = alloc_call(&eval_list_elements_proc, eval_args, alloc_cont(call));
	
	return perform_call(eval_call);
}

void init_function_eval_procedures(void) {
	init_primitive(&eval_primitive_procedure, &eval_primitive_procedure_proc);
	init_primitive(&eval_primitive_procedure_call, &eval_primitive_procedure_call_proc);
	
	init_primitive(&eval_function, &eval_function_proc);
	init_primitive(&eval_function_call, &eval_function_call_proc);
	
	add_list_application_procedure(type_primitive_procedure, &eval_primitive_procedure_proc);
	add_list_application_procedure(type_function, &eval_function_proc);
}
