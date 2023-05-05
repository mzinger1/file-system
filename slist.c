/* A simple linked list of strings. */

#include <alloca.h>
#include <stdlib.h>
#include <string.h>

#include "slist.h"

// Cons the given string to the given string list.
slist_t *s_cons(const char *text, slist_t *rest) {
	slist_t *xs = malloc(sizeof(slist_t));
	xs->data = strdup(text);
	xs->refs = 1;
	xs->next = rest;
	return xs;
}

// Free the given string list.
void s_free(slist_t *xs) {
	if (xs == 0) {
		return;
	}

	xs->refs -= 1;

	if (xs->refs == 0) {
		s_free(xs->next);
		free(xs->data);
		free(xs);
	}
}

// Split the given string on the given delimiter into a list of strings.
slist_t *s_split(const char *text, char delim) {
	if (*text == 0) {
		return 0;
	}

	int plen = 0;
	while (text[plen] != 0 && text[plen] != delim) {
		plen += 1;
	}

	int skip = 0;
	if (text[plen] == delim) {
		skip = 1;
	}

	slist_t *rest = s_split(text + plen + skip, delim);
	char *part = alloca(plen + 2);
	memcpy(part, text, plen);
	part[plen] = 0;

	return s_cons(part, rest);
}
