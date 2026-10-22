/*
  NTEST_TMPPATH() must give each test process its own path and each
  invocation its own buffer, so tests can run in parallel and a path is
  not overwritten by the next NTEST_TMPPATH() call.
*/
#include <unistd.h>

#include "n_check.h"

START_TEST(test_tmp_path)
{
    char pid[64];
    const char *one = NTEST_TMPPATH("one");
    const char *two = NTEST_TMPPATH("two");

    fail_unless(one != two, "two NTEST_TMPPATH() calls aliased");
    fail_unless(strstr(one, "one") != NULL, "first path was overwritten: %s", one);
    fail_unless(strstr(two, "two") != NULL, "second path is wrong: %s", two);

    snprintf(pid, sizeof(pid), "-%ld-", (long)getpid());
    fail_unless(strstr(one, pid) != NULL, "%s has no pid", one);
    fail_unless(strstr(two, pid) != NULL, "%s has no pid", two);
}
END_TEST

NTEST_RUNNER("ncheck", test_tmp_path);
