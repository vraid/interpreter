#pragma once

#include "data-structures.h"

object map_proc;
object filter_proc;
object fold_proc;

void add_map_procedure(object_type type, object* proc);
void add_filter_procedure(object_type type, object* proc);
void add_fold_procedure(object_type type, object* proc);

void init_higher_order_procedures(void);
