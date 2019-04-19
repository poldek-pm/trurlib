#include "test.h"
#include "niobuf.h"

#define CPATH "/tmp/trurlib-n_iobuf-tests-foo.zst"

START_TEST (test_write)
{
    tn_iobuf *io = n_iobuf_open(CPATH, "w");
    FILE *stream = popen("rpm -ql glibc", "r");

    char buf[1024];
    //printf("\n\nCreating %s with rpm -qla...", path);
    //fflush(stdout);

    while (fgets(buf, sizeof(buf), stream))
        n_iobuf_write(io, buf, strlen(buf));

    n_iobuf_close(io);
}
END_TEST


START_TEST (test_read)
{
    char buf[16];
    int n;

    printf("test_read\n");
    tn_iobuf *io = n_iobuf_open(CPATH, "r");

    //memset(buf, '$', sizeof(buf));
    while ((n = n_iobuf_read(io, buf, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        //printf("read %d\n", n);
        ///printf("buf [%s]\n", buf);
    }

    n_iobuf_close(io);
}
END_TEST


START_TEST (test_read_write)
{
    char buf[75];
    int i, nitems = 10;
    char *fmt = "line%.12d";
    int nw = 0, nr = 0;

    tn_iobuf *io = n_iobuf_open(CPATH, "w");
    for (i = 0; i < nitems; i++) {
        int n = snprintf(buf, sizeof(buf), fmt, i);
        printf("write %s %d = %d\n", buf, n, strlen(buf));

        nw += n_iobuf_write(io, buf, n);
    }
    n_iobuf_close(io);

    io = n_iobuf_open(CPATH, "r");
    for (i = 0; i < nitems; i++) {
        int n = snprintf(buf, sizeof(buf), fmt, i);
        char rbuf[75];
        memset(rbuf, sizeof(rbuf), 0);
        int x = n_iobuf_read(io, rbuf, n);
        rbuf[x] = '\0';
        nr += x;
        printf("read %s %d = %d  => %s\n", buf, n, strlen(buf), rbuf);

    }
    n_iobuf_close(io);
    printf("OK\n");
}
END_TEST

START_TEST (test_seek)
{
    char buf[75];
    int nitems = 10;
    char *fmt = "line%.12d";
    int nw = 0, nr = 0;
    tn_iobuf *io;
    char rbuf[75];
    int i, x, n, nth;

    io = n_iobuf_open(CPATH, "w");
    for (i = 0; i < nitems; i++) {
        int n = snprintf(buf, sizeof(buf), fmt, i);
        nw += n_iobuf_write(io, buf, n);
    }
    n_iobuf_close(io);

    io = n_iobuf_open(CPATH, "r");
    n_assert(n_iobuf_tell(io) == 0);

    n_iobuf_seek(io, 16, SEEK_SET);
    n_assert(n_iobuf_tell(io) == 16);

    nth = 1;
    n = snprintf(buf, sizeof(buf), fmt, nth);
    x = n_iobuf_read(io, rbuf, n);
    n_assert(x == n);
    n_assert(memcmp(buf, rbuf, n) == 0);

    n_iobuf_seek(io, 0, SEEK_SET);
    n_assert(n_iobuf_tell(io) == 0);

    nth = 0;
    n = snprintf(buf, sizeof(buf), fmt, nth);
    x = n_iobuf_read(io, rbuf, n);
    printf("n = %d, x = %d\n", n, x);
    n_assert(x == n);
    n_assert(memcmp(buf, rbuf, n) == 0);
    n_assert(n_iobuf_tell(io) == 16);

    nth = 1;
    n = snprintf(buf, sizeof(buf), fmt, nth);
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
}
END_TEST


struct test_suite test_suite_niobuf = {
    "n_iobuf",
    {
        { "write", test_write },
        { "read", test_read   },
        { "read/write", test_read_write  },
        { NULL, NULL }
    }
};
