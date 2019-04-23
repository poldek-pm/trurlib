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
#include "trurl_internal.h"
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

#endif
