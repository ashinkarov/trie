Trie implemetnation in C
========================

This a simple implementation of (trie)[http://en.wikipedia.org/wiki/Trie]
data structure in C99.  This implementation allows to store sequences of
`int`-s, however one can easy trigger type of `symb` in `struct trie`.

The library includes a simple test-case in the trie.c.  In order to make
it work, run `make test`.

Usage
-----

Currently implemetation allows you to store `char` based strings in the
trie, attaching to each word some information using `ssize_t` type.  For
example, consider the following usage:

```C
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "trie.h"

/* some values, we want to attach to every
   word in the trie data base.  */
enum type_of_word {
  type_a, type_b
};

int main (int argc, char *argv[])
{
  int ret = EXIT_SUCCESS;
  ssize_t res;

  /* Initialize new trie.  */
  truct trie *  dict = trie_new ();
  if (argc <= 1)
    {
      (void) fprintf (stderr, "usage: %s word to find\n", argv[0]);
      ret = EXIT_FAILURE;
      goto out;
    }

  /* Add two words of two different types.  */
  trie_add_word (dict, "hello", strlen ("hello"), (ssize_t)type_a);
  trie_add_word (dict, "hell", strlen ("hell"), (ssize_t)type_b);


  printf ("Printing the trie.\n");
  trie_print (dict);

  /* Search the argument in the trie and detect a type of it.  */
  res = trie_search (dict, argv[1], strlen (argv[1]));
  printf ("searching '%s' in the database -- %s\n",
    argv[1],  res != TRIE_NOT_LAST
    ? (enum type_of_word)res == type_a ? "yes, type A" : "yes, type B"
    : "no");

out:
 trie_free (dict);
 return ret;
}

```

As the type of information you attach is a transparent `size_t` we need
to dedicate one symbol, whic would denote that a certain symbol is _not_
last.  Macro `TRIE_NOT_LAST` serves this purpose.

Tha implementation is valgrinded, and is compiled with strictes gcc warning
level.

Send your suggestions or patches.
