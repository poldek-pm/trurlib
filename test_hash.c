/*
   $Id$
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <trurl/trurl.h>


#define TEST_HASH       0

void hash_map_func(const char *key, void *data)
{
    char *k = (char *) key;
    char *d = (char *) data;
    printf("'%s' = '%s'\n", k, d);
}


void test_hash(void)
{
    int i;
    tn_hash *h;

    if ((h = n_hash_new(2048, free)) == NULL)
	exit(1);

    for (i = 0; i < 2048; i++) {
	char str[40];
	sprintf(str, "key%.4d", i);

	n_hash_insert(h, str, strdup("ma kota i psa"));
    }

    n_hash_map(h, hash_map_func);
    n_hash_free(h);
}



int main()
{
    test_hash();
    return 0;
}
