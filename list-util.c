#include "list-util.h"
#include "global-variables.h"

object* cons(object* first, object* rest) {
	object* obj = allocate_list_type(list_type(rest));
	obj->data.list.first = first;
	obj->data.list.rest = rest;
	return obj;
}

object* list_ref(int n, object* ls) {
	while (n > 0) {
		ls = list_rest(ls);
		n--;
	}
	return list_first(ls);
}

object* list_take(int n, object* obj) {
	if (n == 0) {
		return empty_list();
	}
	else {
		object* ls = allocate_list();
		object* prev;
		object* next = ls;
		while (n > 0) {
			prev = next;
			prev->data.list.first = list_first(obj);
			obj = list_rest(obj);
			n--;
			if (n == 0) {
				next = empty_list();
			}
			else {
				next = allocate_list();
			}
			prev->data.list.rest = next;
		}
		return ls;
	}
}

object* list_drop(int n, object* obj) {
	while (n > 0) {
		obj = list_rest(obj);
		n--;
	}
	return obj;
}

object* list_append(object* as, object* rest) {
	if (is_empty_list(as)) {
		return rest;
	}
	else {
		object* ls = allocate_list();
		object* prev;
		object* next = ls;
		while (!is_empty_list(as)) {
			prev = next;
			prev->data.list.first = list_first(as);
			as = list_rest(as);
			if (is_empty_list(as)) {
				next = rest;
			}
			else {
				next = allocate_list();
			}
			prev->data.list.rest = next;
		}
		return ls;
	}
}

object* list_reverse(object* obj) {
	object* ls = empty_list();
	while (!is_empty_list(obj)) {
		ls = cons(list_first(obj), ls);
		obj = list_rest(obj);
	}
	return ls;
}