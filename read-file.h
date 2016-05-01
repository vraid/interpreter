#pragma once

#include "read-file.h"
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "delist.h"

object read_file_proc;

object* read_file(object* args, object* cont) {
	object* input_port;
	delist_1(args, &input_port);
	return no_object();
}

void init_read_file_procedures(void) {
	init_primitive_procedure(&read_file_proc, &read_file);
}
