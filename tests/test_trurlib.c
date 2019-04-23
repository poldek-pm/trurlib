#include <check.h>
#include "test.h"

extern struct test_suite test_suite_niobuf;
extern struct test_suite test_suite_nstream;

struct test_suite *suites[] = {
    &test_suite_niobuf,
    &test_suite_nstream,
    NULL,
};

Suite *make_suite(struct test_suite *tsuite)
{
    Suite *s = suite_create(tsuite->name);
    int i = 0;

    while (tsuite->cases[i].name) {
        TCase *tc = tcase_create(tsuite->cases[i].name);
        tcase_add_test(tc, tsuite->cases[i].test_fn);
        suite_add_tcase(s, tc);
        i++;
    }
    return s;
}

int main(int argc, char *argv[])
{
    int i = 0, nerr = 0;

    //if (argc > 1 && n_str_eq(argv[1], "-v"))
    //    poldek_set_verbose(1);

    while (suites[i]) {
        Suite *s = make_suite(suites[i]);
        SRunner *sr = srunner_create(s);
        srunner_run_all(sr, CK_NORMAL);
        nerr += srunner_ntests_failed(sr);
        srunner_free(sr);
        i++;
    }

    return (nerr == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
