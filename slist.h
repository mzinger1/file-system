/* A simple linked list of strings. */

#ifndef SLIST_H
#define SLIST_H

typedef struct slist {
  char *data;
  int refs;
  struct slist *next;
} slist_t;

// Cons the given string to the given string list.
slist_t *s_cons(const char *text, slist_t *rest);

// Free the given string list.
void s_free(slist_t *xs);

// Split the given string on the given delimiter into a list of strings.
slist_t *s_split(const char *text, char delim);

#endif
