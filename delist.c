#include "delist.h"
#include "data-structures.h"

void delist_1(object* ls, object** first) {
	*first = list_first(ls);
}

void delist_2(object* ls, object** first, object** second) {
	*first = list_first(ls);
	ls = list_rest(ls);
	*second = list_first(ls);
}

void delist_3(object* ls, object** first, object** second, object** third) {
	*first = list_first(ls);
	ls = list_rest(ls);
	*second = list_first(ls);
	ls = list_rest(ls);
	*third = list_first(ls);
}
