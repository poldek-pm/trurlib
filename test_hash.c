/*
   $Id$
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nhash.h"

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

    if ((h = n_hash_new(7, NULL)) == NULL)
	exit(1);

    n_hash_ctl(h, TN_HASH_REHASH);

    for (i = 0; i < 4000; i++) {
	char str[40];
	sprintf(str, "key%.4d", i);

	n_hash_insert(h, str, "ma kota i psa");
        if (!n_hash_exists(h, str))
            printf("ERROR %s not exists\n", str);
    }

    for (i = 0; i < 4000; i++) {
	char str[40];
	sprintf(str, "key%.4d", i);

	if (!n_hash_exists(h, str))
            printf("ERROR %s not exists\n", str);
            
    }

    
    
    //  sleep(5);
//    n_hash_map(h, hash_map_func);
    n_hash_free(h);
}



int main()
{
    test_hash();
    return 0;
}
