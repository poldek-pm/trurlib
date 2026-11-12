#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nstr.h"
#include "n_check.h"

START_TEST(test_strtokl)
{
    const char **tokens, **p;

    tokens = n_str_tokl("ala; ma; kota, i psa", ";, ");

    p = tokens;
    expect_str(*p, "ala");
    p++;
    expect_str(*p, "ma");
    p++;
    expect_str(*p, "kota");
    p++;
    expect_str(*p, "i");
    p++;
    expect_str(*p, "psa");
    p++;
    expect_null(*p);


    tokens = n_str_tokl("ala", ":");
    p = tokens;
    expect_str(*p, "ala");

    p++;
    expect_null(*p);

    n_str_tokl_free(tokens);
}
END_TEST

START_TEST(test_str_len)
{
    unsigned int len;
    char *arr[] = {"ala;", "ma;", " kota,", "i psa", ";, "};
    char *s;

    len = n_str_len(arr[0], arr[1], arr[2], arr[3], arr[4], (char *) NULL);
    s = n_str_concat(arr[0], arr[1], arr[2], arr[3], arr[4], (char *) NULL);

    expect_str(s, "ala;" "ma;" " kota," "i psa" ";, ");
    expect_int(strlen(s), len);

    free(s);
}
END_TEST

START_TEST(test_str_in)
{
    ck_assert(!n_str_in("ala", "ola", "ela", "cela", NULL));
    ck_assert(n_str_in("ala", "ola", "ela", "cela", "ala", NULL));
}
END_TEST

#include <trurl/narray.h>
START_TEST(test_str_etokl)
{
    const char *line = "ala ma kota i psa";
    tn_array *tl;

    tl = n_str_etokl(line);
    expect_int(n_array_size(tl), 5);

    //    for (i=0; i < n_array_size(tl); i++) {
    //    char *s = n_array_nth(tl, i);
    //    printf("%d. %s\n", i, *s ? s : "BREAK");
    //}
    n_array_free(tl);
}
END_TEST

#include <trurl/n_snprintf.h>
START_TEST(test_snprintf)
{
    char buf[16] = "xxxxxxxxxxxxxxx";

    int n = n_snprintf(buf, sizeof(buf), "%s/%s", "ala", "ma");
    expect_int(n, strlen("ala/ma"));
    ck_assert(strcmp(buf, "ala/ma") == 0);

}
END_TEST


START_TEST(test_str_strip_ws)
{
    char s_normal[] = "  ala ma  ";
    char s_empty[] = "";
    char s_blank[] = " \t  ";
    char s_tail[] = "\tala\r\n";
    char s_guard[] = "x ";        /* [0] guard, [1] byte before the string */
    char *p;

    p = n_str_strip_ws(s_normal);
    expect_str(p, "ala ma");
    ck_assert(p == s_normal + 2);       /* leading ws is skipped, not zeroed */

    expect_str(n_str_strip_ws(s_empty), "");
    expect_str(n_str_strip_ws(s_blank), "");
    expect_str(n_str_strip_ws(s_tail), "ala");

    /* The trailing-whitespace loop must stop at the string, not walk
       below it. This is deterministic: an older implementation zeroed
       the space just before the empty string, then stopped at 'x'. */
    p = n_str_strip_ws(s_guard + 2);
    expect_str(p, "");
    ck_assert(s_guard[0] == 'x');
    ck_assert(s_guard[1] == ' ');

    ck_assert(n_str_strip_ws(NULL) == NULL);
}
END_TEST


NTEST_RUNNER("str", test_strtokl, test_str_len, test_str_in, test_str_etokl, test_snprintf,
             test_str_strip_ws);
