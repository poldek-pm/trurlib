/*
   $Id$
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
#include <limits.h>

int cmpNstr(const char *a, const char *b)
{
    return strncmp(a, b, 3);
}


int cmpstr_len(const void *a, const void *b)
{
    return strlen(a) - strlen(b);
}

void print_list(char *msg, const tn_list * l)
{
    char *s;
    tn_list_iterator li;
    
    n_list_iterator_start(l, &li);
    printf("%s", msg);
    while ((s = n_list_iterator_get(&li))) {
	printf("%s, ", s);
    }
    printf("\n");
}

void test_list(void)
{
    int i, n;
    char *s;
    tn_list *l;

    l = n_list_new(0, free, (t_fn_cmp) strcmp);

    for (i = 1; i < 10; i++) {
	char str[10];

	sprintf(str, "%d", i);
	n_list_push(l, strdup(str));

	sprintf(str, "%d", i + 1);

	n_list_push(l, strdup(str));
	n_list_push(l, strdup(str));

	sprintf(str, "%d", i - 1);
	n_list_unshift(l, strdup(str));
	n_list_push(l, strdup(str));
    }

    print_list("list = ", l);


    for (i = 0; i < 20; i++) {
	char str[10];
	sprintf(str, "%d", i);

	printf("removed '%d' =>", i);
	n_list_remove(l, str);

	print_list("", l);

	if (n_list_size(l) == 0)
	    break;

    }

    for (i = 0; i < n_list_size(l); i++) {
	printf("%d:%s\n", i, (char *) n_list_nth(l, i));
    }

    n_list_free(l);

    l = n_list_new(TN_LIST_UNIQ, NULL, (t_fn_cmp) strcmp);
    n_list_push(l, "1");
    n_list_push(l, "2");
    n_list_push(l, "3");
    n_list_push(l, "4");
    n_list_push(l, "5");
    n_list_push(l, "100");
    n_list_push(l, "200");
    n_list_push(l, "300");
    n_list_push(l, "400");
    n_list_push(l, "500");


    n_assert(n_list_push(l, "1") == NULL);


    print_list("list = ", l);

    while (n_list_size(l)) {
	char *s = n_list_pop(l);
	printf("pop %s => ", s);
	print_list("", l);
    }


    n_list_push(l, "1");
    n_list_push(l, "2");
    n_list_push(l, "3");
    n_list_push(l, "4");
    n_list_push(l, "5");

    n_assert(n_list_push(l, "1") == NULL);

    print_list("list = ", l);


    while (n_list_size(l)) {
	char *s = n_list_shift(l);
	printf("shift %s => ", s);
	print_list("", l);
    }


    /*fifo */
    n_list_push(l, "1");
    n_list_push(l, "2");
    n_list_push(l, "3");
    n_list_push(l, "4");
    n_list_push(l, "5");

    n_assert(n_list_push(l, "1") == NULL);

    print_list("\nlist = ", l);

    i = 0;
    while (n_list_size(l)) {
	char *s = n_list_shift(l);
	printf("shift %s, push %s => ", s, s);
	n_list_push(l, s);
	print_list("", l);
	if (i++ == 100)
	    break;
    }

    n_list_push(l, "100");
    n_list_push(l, "200");
    n_list_push(l, "300");
    n_list_push(l, "400");
    n_list_push(l, "500");


    print_list("\n\nlist = ", l);
    while ((n = n_list_size(l))) {

	char *s, *s2;

	if (n < 3)
	    break;


	s = n_list_remove_nth(l, n - 2);
	s2 = n_list_remove_nth(l, n - 3);



	printf("remove %s(%d), %s(%d) => ", s, n - 2, s2, n - 3);
	print_list("", l);
    }
    s = n_list_remove_nth(l, 0);

    print_list("list = ", l);

    s = n_list_pop(l);
    print_list("list = ", l);

    printf("Die test...\n");
    set_trurl_err_hook((void (*)(const char *)) puts);

    puts("shift :");
    n_assert(n_list_shift(l) == NULL);

    puts("pop :");
    n_assert(n_list_pop(l) == NULL);

    puts("remove_nth :");
    n_assert(n_list_remove_nth(l, 100) == NULL);

    n_list_free(l);

}

int main()
{
    printf("\nTEST tn_list\n");
    test_list();
    test_list();
    
    

    return 0;
}
