/*
   $Id$
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "trurl.h"
#undef n_array_push
//#include "nstring.h"


#include <sys/types.h>
#include <limits.h>

int cmpstr_for_libc_qsort(const void *a, const void *b)
{
    const char *s1 = *(const char **) a;
    const char *s2 = *(const char **) b;


    if (s1 != NULL && s2 != NULL) {
	return strcmp(s1, s2);

    } else if (s1 == NULL && s2 == NULL) {
	return 0;

    } else if (s1 != NULL && s2 == NULL) {
	return -1;

    } else if (s1 == NULL && s2 != NULL) {
	return 1;
    }
    n_assert(0);
    return 0;
}


int cmpstr(const void *a, const void *b)
{
    if (a != NULL && b != NULL) {
	return strcmp(a, b);

    } else if (a == NULL && b == NULL) {
	return 0;

    } else if (a != NULL) {
	return -1;

    } else if (b != NULL) {
	return 1;
    }
    n_assert(0);
    return 0;
}


int cmpNstr(const void *a, const void *b)
{
    if (a != NULL && b != NULL) {
	return strncmp(a, b, 3);

    } else if (a == NULL && b == NULL) {
	return 0;

    } else if (a != NULL) {
	return -1;

    } else if (b != NULL) {
	return 1;
    }
    n_assert(0);
    return 0;
}

int cmpstr_len(const void *a, const void *b)
{

    if (a != NULL && b != NULL) {
	return strlen(a) - strlen(b);

    } else if (a == NULL && b == NULL) {
	return 0;

    } else if (a != NULL && b == NULL) {
	return -1;

    } else if (a == NULL && b != NULL) {
	return 1;
    }
    n_assert(0);
    return 0;
}


void test_array_sort_search(void)
{
    char buf[4096];
    int i, n = 0;
    tn_array *a = n_array_new(8, free, (t_fn_cmp) strcmp);
    tn_array *b = n_array_new(8, free, (t_fn_cmp) strcmp);

    while (fgets(buf, 3000, stdin)) {
	strchr(buf, '\n')[0] = '\0';
	n_array_push(a, n_strdup(buf));
	n_array_unshift(a, n_strdup(buf));

	n_array_push(b, n_strdup(buf));
	n_array_unshift(b, n_strdup(buf));
	n++;
    }

    n_array_sort(b);

    n_assert(n_array_eq(b, a));	/* our qsort() works fine? */

    n_array_dump_stats(a, "A");
    n_array_dump_stats(b, "B");

    for (i = 0; i < n_array_size(b); i++) {
	char *p = n_array_nth(b, i);

	printf("%s\n", p);
    }


    n_array_free(a);
    n_array_free(b);
}


/* read lines from stdin */
void test_array_big(void)
{
    char buf[4096];
    int i, n = 0;
    tn_array *a = n_array_new(8, free, (t_fn_cmp) strcmp);
    tn_array *b = n_array_new(8, free, (t_fn_cmp) strcmp);

    printf("\nTEST INTERACTIVE tn_array\n");

    printf("Enter some strings,  C-d to finish\n");

    while (fgets(buf, 3000, stdin)) {
	strchr(buf, '\n')[0] = '\0';
	n_array_push(a, n_strdup(buf));
	n_array_unshift(a, n_strdup(buf));
	n_array_set_nth(b, n + 10, n_strdup(buf));
	n++;
    }
    n_array_dump_stats(a, "A");
    n_array_sort(a);

    for (i = 0; i < n_array_size(a); i++) {
	printf("%d: %s\n", i, (char *) n_array_nth(a, i));
    }

    for (i = 0; i < n_array_size(b); i++) {
	printf("%d: %s\n", i, (char *) n_array_nth(a, i));
    }
}


int strcmp_speed(const void *a, const void *b)
{
    return strcmp(*(const char **) a, *(const char **) b);
}



void array_bsearch_str(tn_array * arr, const char *s)
{
    char *p;

    printf("Looking up %s...", s);

    p = n_array_bsearch_ex(arr, s, cmpNstr);

    if (p != NULL) {
	printf("found (%s) :-)\n", p);

    } else {
	printf("not found :-(\n");
    }
}


void print_array_str(const tn_array * arr, const char *name, const char *sep)
{
    int i, n = n_array_size(arr);

    n_array_dump_stats(arr, name);
    printf("Content[%d]:\n", n_array_size(arr));
    for (i = 0; i < n_array_size(arr); i++) {
	char *p = n_array_nth(arr, i);
	if (p != NULL)
	    printf("[%d]\t = %s%s", i, p, sep ? sep : "\n");
    }
    printf("\n");
}



int test_array_basic(void)
{
    tn_array *arr;
    char *s, *p;
    int i;


    arr = n_array_new(8, free, (t_fn_cmp) strcmp);

    n_array_push(arr, n_strdup("Ala"));
    n_assert(n_array_size(arr) == 1);

    n_array_push(arr, n_strdup("ma"));
    n_assert(n_array_size(arr) == 2);

    n_array_push(arr, n_strdup("kota"));
    n_assert(n_array_size(arr) == 3);
    print_array_str(arr, "arr", NULL);

    s = n_array_pop(arr);
    n_assert(s != NULL);
    n_assert(strcmp(s, "kota") == 0);
    free(s);

    n_assert(n_array_size(arr) == 2);
    print_array_str(arr, "arr", NULL);

    n_array_push(arr, n_strdup("psa"));
    n_assert(n_array_size(arr) == 3);

    print_array_str(arr, "arr", NULL);


    s = n_array_shift(arr);
    n_assert(strcmp(s, "Ala") == 0);
    free(s);

    n_array_unshift(arr, n_strdup("Ola"));
    n_assert(n_array_size(arr) == 3);

    n_array_unshift(arr, n_strdup("Ala i"));
    n_assert(n_array_size(arr) == 4);

    n_array_set_nth(arr, 2, n_strdup("maj±"));
    n_array_set_nth(arr, 3, n_strdup("koty"));
    n_assert(n_array_size(arr) == 4);

    print_array_str(arr, "arr", NULL);

    n_array_set_nth(arr, 100, n_strdup("kot100"));
    n_array_set_nth(arr, 99, n_strdup("kot99"));
    n_array_set_nth(arr, 150, n_strdup("kot150"));
    n_array_set_nth(arr, 10, n_strdup("Ania"));

    printf("\nUnsorted:");
    print_array_str(arr, "arr", "\n");

    n_array_sort_ex(arr, cmpstr);

    printf("\nSorted - ");
    print_array_str(arr, "arr", "\n");


    array_bsearch_str(arr, "Ela");
    array_bsearch_str(arr, "Ania");
    array_bsearch_str(arr, "Ala");


    n_array_sort_ex(arr, (t_fn_cmp) cmpstr_len);
    printf("\nSorted by length:");
    print_array_str(arr, "arr", NULL);


    for (i = 0; i < 2; i++) {
        p = n_array_nth(arr, 0);
        s = n_array_shift(arr);
        n_assert(s == p);

        n_array_push(arr, s);

        p = n_array_nth(arr, n_array_size(arr) - 1);
        s = n_array_pop(arr);
        n_assert(s == p);
        if (s != NULL)
            free(s);
    }


    printf("\nDisplay 3:");
    print_array_str(arr, "arr", NULL);
    set_trurl_err_hook((void (*)(const char *)) puts);
    printf("Remove 3, 4, 10...\n");
    n_array_remove_nth(arr, 3);
    print_array_str(arr, "arr", NULL);
    n_array_remove_nth(arr, 4);
    print_array_str(arr, "arr", NULL);
    n_array_remove_nth(arr, 10);
    print_array_str(arr, "arr", NULL);

    print_array_str(arr, "arr", "\n");

    set_trurl_err_hook((void (*)(const char *)) puts);

    printf("\nShift all: ");
    while (n_array_size(arr)) {
        s = n_array_shift(arr);

        if (s != NULL) {	/* arr contains some NULLs */
            printf("%s, ", s);
            free(s);
        }
    }
    printf("\nNow shift from empty array\n");

    n_assert(n_array_size(arr) == 0);
    s = n_array_shift(arr);
    n_assert(s == NULL);

    printf("Now pop from empty array\n");
    s = n_array_pop(arr);
    n_assert(s == NULL);


    n_array_free(arr);

    return 0;
}


void test_array_growth(void)
{
    tn_array *arr1, *arr2, *arr3;
    char *s1 = "ala ";
    char *s2 = "ma ";
    char *s3 = "kota ";
    int i;

    printf("\nTEST tn_array growth\n");
    arr1 = n_array_new(8, NULL, (t_fn_cmp) strcmp);
    
    arr2 = n_array_new(8, NULL, (t_fn_cmp) strcmp);
    arr3 = n_array_new(8, NULL, (t_fn_cmp) strcmp);
    n_array_ctl(arr3, TN_ARRAY_CONSTSIZE);

    for (i = 0; i < 8; i++) {
	n_array_push(arr1, s1);
	n_array_push(arr1, s2);
	n_array_push(arr1, s3);

	n_array_push(arr2, s1);
	n_array_push(arr2, s2);
	n_array_push(arr2, s3);

	n_array_push(arr3, s1);
    }
    n_array_dump_stats(arr1, "arr1");
    n_array_dump_stats(arr2, "arr2");
    n_array_dump_stats(arr3, "arr3");

    printf("Die test...\n");
    set_trurl_err_hook((void (*)(const char *)) puts);
    n_assert(n_array_push(arr3, s1) == NULL);

    n_array_free(arr1);
    n_array_free(arr2);
    n_array_free(arr3);
}

void test_array_sort(void)
{
    char buf[4096];
    int i, n = 0;
    tn_array *a = n_array_new(8, free, (t_fn_cmp) strcmp);

    for (i=0; i<100; i++) {
        snprintf(buf, sizeof(buf), "%.3d", i);
        n_array_push(a, n_strdup(buf));
    }
    n_array_sort(a);
    for (i = 0; i < n_array_size(a); i++) {
        char *p = n_array_nth(a, i);

        printf("%s\n", p);
    }

    n_array_free(a);
}

void test_array_remove(void)
{
    tn_array *arr;
    char *s1 = "ala ";
    char *s2 = "ma ";
    char *s3 = "kota ";
    int i;

    printf("\nTEST tn_array remove\n");
    arr = n_array_new(14, free, (t_fn_cmp) strcmp);
    for (i = 0; i < 4; i++) {
        n_array_push(arr, n_strdup(s1));
        n_array_push(arr, n_strdup(s2));
        n_array_push(arr, n_strdup(s3));
    }
    n_array_push(arr, n_strdup("dupa"));
    n_array_push(arr, n_strdup("dupa2"));
    n_array_unshift(arr, n_strdup("blada"));
    print_array_str(arr, "start", "\n");

    n_array_remove(arr, "dupa");
    print_array_str(arr, "removed dupa", "\n");

    n_array_remove(arr, "blada");
    print_array_str(arr, "removed blada", "\n");
    
    n_array_remove(arr, s3);
    print_array_str(arr, "removed kota", "\n");
    n_array_remove(arr, s2);
    print_array_str(arr, "removed ma", "\n");
    n_array_remove(arr, s1);
    print_array_str(arr, "removed ala", "\n");

    print_array_str(arr, "aa", "\n");
    n_array_push(arr, n_strdup(s3));
    n_array_remove_nth(arr, 0);
    //n_array_remove(arr, "dupa2");
    print_array_str(arr, "removed dupa2", "\n");
    
    n_array_free(arr);
}


int main()
{

    //test_array_basic();
    //test_array_growth();
    //test_array_basic();
    //test_array_growth();
    test_array_remove();
    /* test_array_big(); */
    //test_array_sort();
    return 0;
}
