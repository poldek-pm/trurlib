/*
   $Id$
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <trurl/trurl.h>

#define TEST_STRTOKL    1
#define TEST_STRLEN     1


void test_strtokl(void)
{
    const char **tokens, **p;

    printf("\nTEST n_str_tokl\n");
    tokens = n_str_tokl("ala; ma; kota, i psa", ";, ");

    p = tokens;

    while (*p) {
        printf("token: %s\n", *p++);
    }
    n_str_tokl_free(tokens);

    printf("2\n");
    tokens = n_str_tokl("alas", ":");
    p = tokens;
    while (*p) {
        printf("token: %s\n", *p++);
    }

    n_str_tokl_free(tokens);
}


void test_strlen(void)
{
    unsigned int len;
    char *arr[] =
    {"ala;", "ma;", " kota,", "i psa", ";, "};
    char *s;

    printf("\nTEST n_str_len\n");

    len = n_str_len(arr[0], arr[1], arr[2], arr[3], arr[4], (char *) NULL);
    s = n_str_concat(arr[0], arr[1], arr[2], arr[3], arr[4], (char *) NULL);

    n_assert(strlen(s) == len);

    printf("length of '%s' = %d\n", s, len);

    free(s);
}


#include <trurl/narray.h>
void test_(char *line)
{
    tn_array *tl;
    int i;

    printf("\nTEST n_str_etokl (%s)\n", line);
    tl = n_str_etokl(line);
    for (i=0; i < n_array_size(tl); i++) {
        char *s = n_array_nth(tl, i);
        printf("%d. %s\n", i, *s ? s : "BREAK");
    }
    
}


int main(int argc, char *argv[])
{
    if (argc > 1) {
        char *s = n_strdup(argv[1]);
        printf("'%s' => '%s'\n", argv[1], n_str_strip_ws(s));
    }
    
//    test_(argv[1]);
#undef TEST_STRTOKL
#define TEST_STRTOKL 0
#if TEST_STRTOKL
    test_strtokl();
#endif

#if TEST_STRLEN
    test_strlen();
#endif
    
    return 0;
}
