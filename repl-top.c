#include "repl-top.h"
#include "allocation.h"
#include "object-init.h"
#include "eval.h"
#include "print.h"
#include "delist.h"
#include "call.h"

void repl_entry(object* environment) {
	/*
	object* stdin_port;
	
	init_object(location_stack, type_file_port, &stdin_port);
	stdin_port.data.file_port.file = stdin;
	
	object repl_entry_cont	
	
	object cont;
	object call;
	
	object args[2];
	init_list_2(args, environment, &stdin_port);
	
//	init_cont(&cont, no_call());
	init_call(&call, &read_entry_proc, args, &cont);
	
	// call read_entry
	top_call(&call);
	*/
}

void init_repl_procedures(void) {
}

// repl top
// start read
// consume whitespace
// read one
// finished reading value: eval, print, back to start read
// finished reading definition: eval, back to start read
// read or eval error: flush input, back to start read