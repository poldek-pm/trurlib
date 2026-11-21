/*
  A failed allocation ends the process with exit code 1, also when the
  hook set with n_malloc_set_failhook() returns: n_malloc(), n_calloc()
  and n_realloc() never return NULL, as their callers take the pointer.
*/
#include <stdint.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "nmalloc.h"
#include "n_check.h"

enum { MALLOC, CALLOC, REALLOC };

static int hook_fd = -1;
static void *block;     /* still reachable when the process exits in n_realloc() */

static void report_only(void)
{
    if (write(hook_fd, "h", 1) != 1)
        _exit(3);
}

/* an allocation no malloc() gives, made in a child with a hook that
   returns; its exit code (2: NULL was returned) and whether the hook ran */
static int fail_in_child(int what, int *hook_ran)
{
    int fds[2], status;
    pid_t pid;
    char c;

    fail_unless(pipe(fds) == 0, "pipe: %m");
    pid = fork();
    fail_unless(pid >= 0, "fork: %m");
    if (pid == 0) {
        void *p;

        close(fds[0]);
        hook_fd = fds[1];
        n_malloc_set_failhook(report_only);
        if (what == MALLOC)
            p = n_malloc(SIZE_MAX);
        else if (what == CALLOC)
            p = n_calloc(SIZE_MAX, 2);
        else {
            block = n_malloc(1);
            p = n_realloc(block, SIZE_MAX);
        }
        _exit(p == NULL ? 2 : 4);
    }

    close(fds[1]);
    *hook_ran = read(fds[0], &c, 1) == 1;
    close(fds[0]);
    fail_unless(waitpid(pid, &status, 0) == pid, "waitpid: %m");
    fail_unless(WIFEXITED(status), "child status %#x", status);
    return WEXITSTATUS(status);
}

static void check_exits(int what)
{
    int hook_ran, rc;

    rc = fail_in_child(what, &hook_ran);
    expect_int(hook_ran, 1);
    expect_int(rc, 1);
}

START_TEST(test_malloc_fails)
{
    check_exits(MALLOC);
}
END_TEST

START_TEST(test_calloc_fails)
{
    check_exits(CALLOC);
}
END_TEST

START_TEST(test_realloc_fails)
{
    check_exits(REALLOC);
}
END_TEST

NTEST_RUNNER("malloc", test_malloc_fails, test_calloc_fails, test_realloc_fails);
