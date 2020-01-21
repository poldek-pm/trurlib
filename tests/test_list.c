#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nassert.h"

#include "n_check.h"
#include "nlist.h"
#include "n_snprintf.h"

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


static const char *list_str(const tn_list *l)
{
    static char buf[4096];
    int n = 0;
    char *s;
    tn_list_iterator li;

    n_list_iterator_start(l, &li);

    while ((s = n_list_iterator_get(&li))) {
        n += n_snprintf(&buf[n], sizeof(buf) - n, "%s,", s);
    }
    return buf;
}

START_TEST(test_list_base)
{
    int i;
    tn_list *l;

    l = n_list_new(0, free, (t_fn_cmp) strcmp);
    for (i = 0; i < 5; i++) {
	char str[10];

	sprintf(str, "%d", i);
	n_list_push(l, strdup(str));
    }
    expect_int(n_list_size(l), 5);
    expect_str(list_str(l), "0,1,2,3,4,");

    n_list_remove(l, "3");
    expect_str(list_str(l), "0,1,2,4,");

    expect_str(n_list_pop(l), "4");
    expect_str(n_list_shift(l), "0");
    expect_str(n_list_shift(l), "1");
    expect_int(n_list_size(l), 1);

    n_list_push(l, "foo");

    expect_str(n_list_shift(l), "2");
    expect_str(n_list_shift(l), "foo");

    expect_int(n_list_size(l), 0);
    n_list_free(l);
}
END_TEST

START_TEST(test_list_uniq)
{
    tn_list *l = n_list_new(TN_LIST_UNIQ, NULL, (t_fn_cmp) strcmp);
    n_list_push(l, "1");
    n_list_push(l, "2");
    n_list_push(l, "3");
    n_list_push(l, "4");
    n_list_push(l, "5");

    expect_null(n_list_push(l, "1"));
    n_list_free(l);
}
END_TEST


NTEST_RUNNER("list", test_list_base, test_list_uniq);
