/*
   $Id$
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nstream.h"
#include "niobuf.h"
#include "nassert.h"
#include "nmalloc.h"

void test_write(const char *path, const char *mode)
{
    char buf[1024];
    tn_iobuf *io = n_iobuf_open(path, mode);

    FILE *stream = popen("rpm -ql glibc", "r");

    printf("\n\nCreating %s with rpm -qla...", path);
    fflush(stdout);

    while (fgets(buf, sizeof(buf), stream))
        n_iobuf_write(io, buf, strlen(buf));

    n_iobuf_close(io);
}


void test_read(const char *path)
{
    char buf[16];
    int n;

    printf("test_read\n");
    tn_iobuf *io = n_iobuf_open(path, "r");

    memset(buf, '$', sizeof(buf));
    while ((n = n_iobuf_read(io, buf, sizeof(buf) - 2)) > 0) {
        buf[n] = '\0';
        printf("buf [%s]\n", buf);
    }

    n_iobuf_close(io);
}

void test_rw(const char *path)
{
    char buf[75];
    int i, nitems = 10;
    char *fmt = "line%.12d";
    int nw = 0, nr = 0;
    tn_iobuf *io;

    io = n_iobuf_open(path, "w");
    for (i = 0; i < nitems; i++) {
        int n = snprintf(buf, sizeof(buf), fmt, i);
        printf("write %s %d = %d\n", buf, n, strlen(buf));

        nw += n_iobuf_write(io, buf, n);
    }
    n_iobuf_close(io);
    printf("%d bytes written\n", nw);

    io = n_iobuf_open(path, "r");
    for (i = 0; i < nitems; i++) {
        int n = snprintf(buf, sizeof(buf), fmt, i);
        char rbuf[75];
        //memset(rbuf, sizeof(rbuf), 0);
        int x = n_iobuf_read(io, rbuf, n);
        rbuf[x] = '\0';
        nr += x;
        printf("read %s %d = %d  => %s\n", buf, n, strlen(buf), rbuf);
    }
    printf("%d bytes read\n", nr);
    int x = n_iobuf_read(io, buf, sizeof(buf));
    printf("%d bytes read %d\n", nr, x);
    n_iobuf_close(io);
}


void test_seek(const char *path)
{
    char buf[75];
    int nitems = 10;
    char *fmt = "line%.12d";
    int nw = 0, nr = 0;
    tn_iobuf *io;
    char rbuf[75];
    int i, x, n;

    io = n_iobuf_open(path, "w");
    for (i = 0; i < nitems; i++) {
        int n = snprintf(buf, sizeof(buf), fmt, i);
        nw += n_iobuf_write(io, buf, n);
    }
    n_iobuf_close(io);

    io = n_iobuf_open(path, "r");
    n_assert(n_iobuf_tell(io) == 0);

    n_iobuf_seek(io, 16, SEEK_SET);
    n_assert(n_iobuf_tell(io) == 16);

    i = 1;
    n = snprintf(buf, sizeof(buf), fmt, i);
    x = n_iobuf_read(io, rbuf, n);
    n_assert(x == n);
    n_assert(memcmp(buf, rbuf, n) == 0);

    n_iobuf_seek(io, 0, SEEK_SET);
    n_assert(n_iobuf_tell(io) == 0);

    i = 0;
    n = snprintf(buf, sizeof(buf), fmt, i);
    x = n_iobuf_read(io, rbuf, n);
    printf("n = %d, x = %d\n", n, x);
    n_assert(x == n);
    n_assert(memcmp(buf, rbuf, n) == 0);
    n_assert(n_iobuf_tell(io) == 16);

    i = 1;
    n = snprintf(buf, sizeof(buf), fmt, i);
    x = n_iobuf_read(io, rbuf, n);
    printf("n = %d, x = %d\n", n, x);
    n_assert(x == n);
    n_assert(memcmp(buf, rbuf, n) == 0);
    n_assert(n_iobuf_tell(io) == 32);

    n_iobuf_seek(io, 16, SEEK_SET);
    n_assert(n_iobuf_tell(io) == 16);

    x = n_iobuf_read(io, rbuf, n);
    printf("n = %d, x = %d\n", n, x);
    n_assert(x == n);
    n_assert(memcmp(buf, rbuf, n) == 0);


    rbuf[x] = '\0';
    nr += x;
    printf("read %s %d = %d  => %s\n", buf, n, strlen(buf), rbuf);
    n_iobuf_close(io);
    return;
    for (i = 0; i < nitems; i++) {
        int n = snprintf(buf, sizeof(buf), fmt, i);
        char rbuf[75];
        //memset(rbuf, sizeof(rbuf), 0);
        x = n_iobuf_read(io, rbuf, n);
        rbuf[x] = '\0';
        nr += x;
        printf("read %s %d = %d  => %s\n", buf, n, strlen(buf), rbuf);
    }
    printf("%d bytes read\n", nr);
    x = n_iobuf_read(io, buf, sizeof(buf));
    printf("%d bytes read %d\n", nr, x);
    n_iobuf_close(io);
}

int main(int argc, char *argv[])
{
    //if (argc > 1) {
    //test_write("/tmp/foo.zst", "w");
    test_seek("/tmp/foo.zst");
        //}

    return 0;
}
