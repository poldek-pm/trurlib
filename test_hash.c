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
    int i, nn = 400000;
    tn_hash *h;
    
    if ((h = n_hash_new(1000000, NULL)) == NULL)
        exit(1);

    //n_hash_ctl(h, TN_HASH_REHASH);
    
    for (i = 0; i < nn; i++) {
        char str[40];
        sprintf(str, "key%d", i);
        if (i % 10000 == 0)
            printf("%s\n", str);
        n_hash_insert(h, str, "ma kota i psa");
        if (!n_hash_exists(h, str))
            printf("ERROR %s not exists\n", str);
    }

    for (i = 0; i < nn; i++) {
        char str[40];
        sprintf(str, "key%d", i);
        if (i % 10000 == 0)
            printf("get %s\n", str);
        if (!n_hash_exists(h, str))
            printf("ERROR %s not exists\n", str);
        
        
    }
    n_hash_stats(h);
    n_hash_free(h);
}


void test_hash_rpmqal(void)
{
    tn_hash *h;
    char buf[1024];
    FILE *f;
    
    if ((h = n_hash_new(300000, NULL)) == NULL)
        exit(1);

    
    //n_hash_ctl(h, TN_HASH_REHASH);
    f = popen("rpm -qal | sort -u", "r");
    
    while (fgets(buf, sizeof(buf), f)) {
        buf[sizeof(buf) - 1] = '\0';
        n_hash_insert(h, buf, "ma kota i psa");
        if (!n_hash_exists(h, buf))
            printf("ERROR %s not exists\n", buf);
    }
    pclose(f);
    
    //  sleep(5);
//    n_hash_map(h, hash_map_func);
    n_hash_stats(h);
    n_hash_free(h);
}



int main()
{
    test_hash_rpmqal();
    test_hash();
    
    return 0;
}
