#pragma once

#include "data-structures.h"

object make_list_proc;
object add_to_list_proc;
object reverse_list_proc;
object list_append_proc;
object list_append_first_reversed_proc;
object unzip_2_proc;

void init_list_util_procedures(void);

int list_length(object* ls);
object* list_ref(int n, object* ls);
