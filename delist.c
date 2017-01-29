#include "delist.h"
#include "data-structures.h"
#include "global-variables.h"

void check_rest_empty(object* ls) {
	if (!is_empty_list(list_rest(ls))) {
		fprintf(stderr, "list too long\n");
	}
}

void delist_1(object* ls, object** a) {
	*a = list_first(ls);
	check_rest_empty(ls);
}

void delist_2(object* ls, object** a, object** b) {
	*a = list_first(ls);
	delist_1(list_rest(ls), b);
}

void delist_3(object* ls, object** a, object** b, object** c) {
	*a = list_first(ls);
	delist_2(list_rest(ls), b, c);
}

void delist_4(object* ls, object** a, object** b, object** c, object** d) {
	*a = list_first(ls);
	delist_3(list_rest(ls), b, c, d);
}

void delist_5(object* ls, object** a, object** b, object** c, object** d, object** e) {
	*a = list_first(ls);
	delist_4(list_rest(ls), b, c, d, e);
}
