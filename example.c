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

  struct trie *  dict = trie_new ();
  if (argc <= 1)
    {
      (void) fprintf (stderr, "usage: %s word to find\n", argv[0]);
      ret = EXIT_FAILURE;
      goto out;
    }

  trie_add_word (dict, "hello", strlen ("hello"), (ssize_t)type_a);
  trie_add_word (dict, "hell", strlen ("hell"), (ssize_t)type_b);


  printf ("Printing the trie.\n");
  trie_print (dict);

  res = trie_search (dict, argv[1], strlen (argv[1]));
  printf ("searching '%s' in the database -- %s\n",
	  argv[1],  res != TRIE_NOT_LAST
	  ? (enum type_of_word)res == type_a ? "yes, type A" : "yes, type B"
	  : "no");

out:
 trie_free (dict);
 return ret;
}
