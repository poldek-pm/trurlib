/*
  TRURLib

  libcheck helpers
*/
#ifndef TRURLIB_N_CHECK_H
#define TRURLIB_N_CHECK_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>          /* for PATH_MAX */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <check.h>
#include "nassert.h"

#define expect_int(a, b)  ck_assert_int_eq(a, b)
#define expect_str(a, b)  ck_assert_str_eq(a, b)
#define expect_null(a)       ck_assert_ptr_null(a)
#define expect_notnull(a)    ck_assert_ptr_nonnull(a)

static inline char *__ncheck_make_test_tmp_path(const char *filename)
{
    static char path[PATH_MAX];
    const char *dir = getenv("TMPDIR");
    if (dir == NULL || *dir == '\0')
        dir = getenv("TMP");

    if (dir == NULL || *dir == '\0')
        dir = "/tmp";

    snprintf(path, sizeof(path), "%s/test_n_check-%s", dir, filename);
    path[sizeof(path) - 1] = '\0';
    return path;
}

#define NTEST_TMPPATH(name) __ncheck_make_test_tmp_path(name)

// Overloading Macro on Number of Arguments
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

static int __ncheck_verbosity = CK_ENV;
static int __ncheck_log_verbose = 0;

static inline void __ncheck_log(const char *fmt, ...)
{
    if (__ncheck_log_verbose < 2) /* enabled with -vv */
        return;

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
#define NTEST_LOG(fmt, args...)  __ncheck_log("[%s] " fmt, __FUNCTION__ , ## args)

static inline int __ncheck_runner(Suite *suite) {
    int nerr = 0;
    SRunner *sr = srunner_create(suite);
    srunner_run_all(sr, __ncheck_verbosity);
    nerr += srunner_ntests_failed(sr);
    srunner_free(sr);

    return (nerr == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

static inline void __ncheck_process_args(int argc, char *argv[], char *tests[]) {

    if (argc < 2)
        return;

    for( int i = 1; i < argc; i++) {
        const char *arg = argv[i];

        if (strcmp(arg, "-l") == 0) {
            int n = 0;
            while (tests[n])
                printf("%s\n", tests[n++]);

            exit(EXIT_SUCCESS);
        }

        if (strncmp(arg, "-v", 2) == 0) {
            __ncheck_verbosity = CK_VERBOSE;
            __ncheck_log_verbose++;
            arg += 2;
            while (*arg++ == 'v')
                __ncheck_log_verbose++;
        }
    }
}

static inline const char *__ncheck_test_name(const char *name) {
    if (strncmp(name, "test_", 5) == 0)
        return name + 5;

    return name;
}


/* internal macro */
#define __REGISTER_TEST(function)             \
    do {                                      \
        n_assert(__suite != NULL);            \
        TCase *tc = tcase_create(__ncheck_test_name(#function)); \
        tcase_add_test(tc, function);         \
        suite_add_tcase(__suite, tc);         \
        int i = 0;                            \
        while (__tests[i] != NULL) i++;       \
        __tests[i] = #function;               \
    } while (0);



#define NTEST_RUNNER(name, ...)                               \
    int main(int argc, char *argv[]) {                        \
        Suite *__suite = suite_create(name);                  \
        char  **__tests = calloc(1024, sizeof(*__tests));     \
        GET_MACRO(_0, __VA_ARGS__, FE_9, FE_8, FE_7, FE_6, FE_5, FE_4, FE_3, FE_2, FE_1, FE_0)(__REGISTER_TEST, __VA_ARGS__); \
        __ncheck_process_args(argc, argv, __tests);           \
        return __ncheck_runner(__suite);                      \
    }
#endif


/* redefine (deprecated) fail_if/fail_unless with variable arguments */
#undef fail_if
#undef fail_unless

#define fail_if_1(cond) ck_assert_msg(!(cond), NULL)
#define fail_if_2p(cond, ...) ck_assert_msg(!(cond), __VA_ARGS__)

#define fail_unless_1(cond) ck_assert_msg((cond), NULL)
#define fail_unless_2p(cond, ...) ck_assert_msg((cond), __VA_ARGS__)

#define fail_if(cond, ...) \
    GET_MACRO(_0, ##__VA_ARGS__, \
        fail_if_2p, fail_if_2p, fail_if_2p, fail_if_2p, fail_if_2p, \
        fail_if_2p, fail_if_2p, fail_if_2p, fail_if_2p, fail_if_1)(cond, ##__VA_ARGS__)

#define fail_unless(cond, ...) \
    GET_MACRO(_0, ##__VA_ARGS__, \
        fail_unless_2p, fail_unless_2p, fail_unless_2p, fail_unless_2p, fail_unless_2p, \
        fail_unless_2p, fail_unless_2p, fail_unless_2p, fail_unless_2p, fail_unless_1)(cond, ##__VA_ARGS__)
