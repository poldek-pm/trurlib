#ifndef TRURLIB_TEST_H
#define TRURLIB_TEST_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>          /* for PATH_MAX */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "nassert.h"
#include "nmalloc.h"
//#include "trurl_internal.h"
#include <check.h>

#define fail_ifnot fail_unless

struct test_case {
    const char *name;
    void (*test_fn)(int);
};

struct test_suite {
    const char *name;
    struct test_case cases[];
};

static char _tmppathbuf[1024];
static char *make_test_tmp_path(char *path, const char *filename)
{
    const char *dir = getenv("TMPDIR");
    if (dir == NULL || *dir == '\0')
        dir = getenv("TMP");

    if (dir == NULL || *dir == '\0')
        dir = "/tmp";

    snprintf(path, 1024, "%s/trurlib-tests-%s", dir, filename);
    path[1024 - 1] = '\0';
    return path;
}

#define TMPPATH(name) make_test_tmp_path(_tmppathbuf, name)

static Suite *__suite = NULL;

#define MAKE_SUITE(name) __suite = suite_create(name)

#define REGISTER_TEST(function)          \
    do {                                 \
        n_assert(__suite != NULL);       \
        TCase *tc = tcase_create( #function); \
        tcase_add_test(tc, function);    \
        suite_add_tcase(__suite, tc);    \
    } while (0);

#define RUN                              \
    int __nerr = 0;                      \
    do {                                 \
        SRunner *sr = srunner_create(__suite); \
        srunner_run_all(sr, CK_NORMAL);        \
        __nerr += srunner_ntests_failed(sr);   \
        srunner_free(sr);                      \
    } while (0);                               \
    return (__nerr == 0) ? EXIT_SUCCESS : EXIT_FAILURE


// taken from https://stackoverflow.com/a/11994395
#define FE_0(WHAT)
#define FE_1(WHAT, X) WHAT(X)
#define FE_2(WHAT, X, ...) WHAT(X)FE_1(WHAT, __VA_ARGS__)
#define FE_3(WHAT, X, ...) WHAT(X)FE_2(WHAT, __VA_ARGS__)
#define FE_4(WHAT, X, ...) WHAT(X)FE_3(WHAT, __VA_ARGS__)
#define FE_5(WHAT, X, ...) WHAT(X)FE_4(WHAT, __VA_ARGS__)
#define FE_6(WHAT, X, ...) WHAT(X)FE_5(WHAT, __VA_ARGS__)
#define FE_7(WHAT, X, ...) WHAT(X)FE_6(WHAT, __VA_ARGS__)
#define FE_8(WHAT, X, ...) WHAT(X)FE_7(WHAT, __VA_ARGS__)
#define FE_9(WHAT, X, ...) WHAT(X)FE_8(WHAT, __VA_ARGS__)
#define GET_MACRO(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, NAME, ...) NAME

#define TEST_RUNNER(name, ...) \
    int main(void) {           \
        MAKE_SUITE(name);      \
        GET_MACRO(_0,__VA_ARGS__,FE_9, FE_8, FE_7, FE_6, FE_5, FE_4, FE_3, FE_2, FE_1, FE_0)(REGISTER_TEST, __VA_ARGS__); \
        RUN;                    \
    }

#endif
