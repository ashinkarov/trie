/* Copyright (c) 2012 Artem Shinkarov <artyom.shinkaroff@gmail.com>

   Permission to use, copy, modify, and distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.
   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "trie.h"

/* Allocate a new empty trie.  */
struct trie *
trie_new ()
{
  struct trie *  trie = (struct trie *) malloc (sizeof (struct trie));
  trie->children_size = TRIE_CHILDREN;
  trie->children_count = 0;
  trie->children = (struct child *)
		   malloc (TRIE_CHILDREN * sizeof (struct child));
  memset (trie->children, 0, TRIE_CHILDREN * sizeof (struct child));
  return trie;
}


/* Helper for bsearch and qsort.  */
static inline int
cmp_children (const void *  k1, const void *  k2)
{
  struct child *  c1 = (struct child *)k1;
  struct child *  c2 = (struct child *)k2;
  return c1->symb - c2->symb;
}


/* Search for a symbol in a children of a certain trie.  Uses binary search
   as the children are kept sorted.  */
static struct child *
trie_search_child (struct trie * trie, int symb)
{
  struct child s;

  if (trie->children_count == 0)
    return NULL;

  s.symb = symb;
  return  (struct child *)bsearch (&s, trie->children, trie->children_count,
			   sizeof (struct child), cmp_children);
}

/* Add a word to the trie.  */
void
trie_add_word (struct trie * trie, const char * word, size_t length, ssize_t info)
{
  struct child *  child;
  struct trie *  nxt = NULL;

  assert (trie != NULL);

  child = trie_search_child (trie, word[0]);

  if (child)
    {
      if (length == 1)
	child->last = info;
      if (length > 1 && child->next == NULL)
	child->next = trie_new ();

      nxt = child->next;
    }
  else
    {
      if (trie->children_count >= trie->children_size)
	{
	  trie->children_size *= 2;
	  trie->children = (struct child *)
			   realloc (trie->children,
				    trie->children_size
				    * sizeof (struct child));
	}

      trie->children[trie->children_count].symb = word[0];
      if (length > 1)
	{
	  trie->children[trie->children_count].next = trie_new ();
	  trie->children[trie->children_count].last = TRIE_NOT_LAST;
	}
      else
	{
	  trie->children[trie->children_count].next = NULL;
	  trie->children[trie->children_count].last = info;
	}

      nxt = trie->children[trie->children_count].next;
      trie->children_count++;

      /* XXX This qsort may not perform ideally, as actually we are always
	 just shifting a number of elements a the end of the array one
	 element to the left.  Possibly qsort, can figure it out and work
	 in O (N) time.  Otherwise better alternative is needed.  */
      qsort (trie->children, trie->children_count,
	     sizeof (struct child), cmp_children);
    }

  if (length > 1)
    trie_add_word (nxt, &word[1], length - 1, info);
}


#define tab(n)			  \
do {				  \
  int __i;			  \
  for (__i = 0; __i < n; __i++)	  \
    printf ("  ");		  \
} while (0)

/* Print the trie.  */
static void
_trie_print (struct trie *  t, int level)
{
  unsigned int i;
  if (!t)
    return;

  for (i = 0; i < t->children_count; i++)
    {
      tab (level);
      printf ("%c %s\n", (char) t->children[i].symb,
	      t->children[i].last != TRIE_NOT_LAST ? "[last]" : "");
      _trie_print (t->children[i].next, level+1);
    }
}

/* Wrapper for print.  */
void
trie_print (struct trie *  t)
{
  _trie_print (t, 0);
}


/* Deallocate memory used for trie.  */
void
trie_free (struct trie *  trie)
{
  unsigned int  i;
  if (!trie)
    return;

  for (i = 0; i < trie->children_count; i++)
    trie_free (trie->children[i].next);

  if (trie->children)
    free (trie->children);
  free (trie);
}

/* Search for word in trie.  Returns true/false.  */
ssize_t
trie_search (struct trie *  trie, const char *  word, size_t length)
{
  struct child *  child;

  assert (length > 0);
  if (trie == NULL)
    return TRIE_NOT_LAST;

  child = trie_search_child (trie, word[0]);

  if (!child)
    return TRIE_NOT_LAST;

  if (length == 1)
    return child->last;
  else
    return trie_search (child->next, &word[1], length - 1);
}

/* Searches if a given word can be found in the database and if a trie follows
   it.  Returns trie, which follows a given prefix, and sets LAST to true if
   the word itself can be found inside the trie.  */
struct trie *
trie_check_prefix (struct trie *  trie, const char *  word, size_t length,
		   ssize_t *  last)
{
  struct child *  child;

  assert (length > 0);
  if (trie == NULL)
    {
      *last = TRIE_NOT_LAST;
      return NULL;
    }

  child = trie_search_child (trie, word[0]);

  if (!child)
    {
      *last = TRIE_NOT_LAST;
      return NULL;
    }

  if (length == 1)
    {
      *last = child->last;
      return child->next;
    }
  else
    return trie_check_prefix (child->next, &word[1], length - 1, last);
}


#ifdef TRIE_MAIN
#include <stdbool.h>

int
main (int argc, char *argv[])
{
  struct trie *  t = trie_new ();
  bool check_search = true, check_prefix_search = false;

  assert (argc > 1);

  #define add_word(t, word)  trie_add_word (t, word, strlen (word), 1)
  add_word (t, "+");
  add_word (t, "++");
  add_word (t, "+=");
  add_word (t, "+++");
  add_word (t, "-+-");
  add_word (t, "+=+");
  add_word (t, "=+=");
  add_word (t, "===");
  add_word (t, "---");
  add_word (t, "+-+");

  printf ("Printing all the database.\n");
  trie_print (t);

  if (check_search)
      printf ("searching '%s' in the database -- %s\n",
	      argv[1], trie_search (t, argv[1],
	      strlen (argv[1])) != TRIE_NOT_LAST ? "yes" : "no");
  else if (check_prefix_search)
    {
      struct trie *  res;
      ssize_t last;
      res = trie_check_prefix (t, argv[1], strlen (argv[1]), &last);
      printf ("checking prefix for '%s' in database last: %s, follows:\n",
	      argv[1], last != TRIE_NOT_LAST ? "yes" : "no");
      trie_print (res);
    }

  trie_free (t);

  return EXIT_SUCCESS;
}
#endif
