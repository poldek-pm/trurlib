/*
   $Id$
 */

/*
   NOTE: unlinks && creates file /tmp/trurl_test.db
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "nassert.h"
#include "xmalloc.h"

#include <trurl/trurl.h>
#include "nstring.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define TEST_DBFILE "/tmp/trurl_test.db"

void dbhash_map_func(const void *key, const void *data, size_t data_size)
{
    char *k = (char *) key;
    char *d = (char *) data;
    printf("'%s'\t=\t'%s'\t(size = %d)\n", k, d, data_size);
}

#define INS_STR_LITERAL(h, key, s) n_dbhash_insert(h, key, s, strlen(s)+1)

void test_dbhash(void)
{
    tn_dbhash *h;

    printf("\nTEST n_dbhash\n");

    unlink(TEST_DBFILE);

    h = n_dbhash_new(TEST_DBFILE, O_CREAT | O_RDWR);

    if (h == NULL) {
	perror("dbopen");
	exit(1);
    }
    INS_STR_LITERAL(h, "ala", "ma kota");
    INS_STR_LITERAL(h, "ela", "ma psa");
    INS_STR_LITERAL(h, "józia", "ma chomika");
    INS_STR_LITERAL(h, "zenek", "ma akwarium");
    INS_STR_LITERAL(h, "ja¶", "ma nic");

    n_dbhash_free(h);

    h = n_dbhash_new(TEST_DBFILE, O_RDONLY);

    n_dbhash_map(h, dbhash_map_func);
    n_dbhash_free(h);

}

void test_dbhash_2(void)
{
    int i;
    tn_dbhash *h;
    const struct ndbhash_data *dt;
    const char *s;


    printf("\nTEST n_dbhash 2\n");
    unlink(TEST_DBFILE);

    h = n_dbhash_new_ex(TEST_DBFILE, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, NULL);

    if (h == NULL) {
	perror("dbopen");
	exit(1);
    }
    
    for (i = 0; i < 2048; i++) {
	char key[20], value[40];

	sprintf(key, "key%.6d", i);
	sprintf(value, "value%.6d", i);

	n_dbhash_insert(h, key, value, sizeof(value));
    }


    /* get */
    dt = n_dbhash_get(h, "key000001");
    n_assert(dt != NULL);
    printf("get: key000001 = (%s, %d)\n", (char *) dt->data, dt->size);

    s = n_dbhash_get_data(h, "key000001");
    printf("get_data: key000001 = (%s)\n", s);


    dt = n_dbhash_get(h, "key001001");
    n_assert(dt != NULL);
    printf("get: key001001 = (%s, %d)\n", (char *) dt->data, dt->size);

    s = n_dbhash_get_data(h, "key001001");
    printf("get_data: key001001 = (%s)\n", s);

    dt = n_dbhash_get(h, "ala ma kota");
    n_assert(dt == NULL);

    s = n_dbhash_get_data(h, "ala ma kota");
    n_assert(s == NULL);

    n_dbhash_free(h);

    
    
    h = n_dbhash_new(TEST_DBFILE, O_RDONLY);
    if (h == NULL) {
	perror("dbopen");
	exit(1);
    }

    n_dbhash_replace(h, "key000001", "aaaaa", 6);
    
    dt = n_dbhash_get(h, "key000001");
    n_assert(dt != NULL);
    printf("\nget: key000001 = (%s, %d)\n", (char *) dt->data, dt->size);

    dt = n_dbhash_get(h, "key000001");
    n_assert(dt != NULL);
    printf("\nget: key000001 = (%s, %d)\n", (char *) dt->data, dt->size);
    
    printf("List all\n");

    h = n_dbhash_new(TEST_DBFILE, O_RDONLY);

    n_dbhash_map(h, dbhash_map_func);
    n_dbhash_free(h);
}


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

    printf("\nTEST tn_hash\n");

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
    test_dbhash();
    test_dbhash_2();
    return 0;
}
