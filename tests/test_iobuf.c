#include "n_check.h"
#include "niobuf.h"

START_TEST (test_read_write)
{
    char buf[75];
    int i, nitems = 10;
    char *fmt = "line%.12d";
    int nw = 0, nr = 0;
    char *tmppath = NTEST_TMPPATH("niobuf-rw.zst");
    tn_iobuf *io = n_iobuf_open(tmppath, "w");

    for (i = 0; i < nitems; i++) {
        int n = snprintf(buf, sizeof(buf), fmt, i);
        nw += n_iobuf_write(io, buf, n);
    }
    n_iobuf_close(io);

    io = n_iobuf_open(tmppath, "r");
    for (i = 0; i < nitems; i++) {
        int n = snprintf(buf, sizeof(buf), fmt, i);
        char rbuf[75];

        memset(rbuf, 0, sizeof(rbuf));

        int x = n_iobuf_read(io, rbuf, n);
        expect_int(x, n);
        expect_int(memcmp(buf, rbuf, n), 0);
        nr += x;
        //printf("read %s %d = %d  => %s\n", buf, n, strlen(buf), rbuf);

    }
    n_iobuf_close(io);
    expect_int(nw, nr);
    //printf("OK\n");
}
END_TEST

void do_seek(const char *path)
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
    expect_int(n_iobuf_tell(io), 0);

    n_iobuf_seek(io, 16, SEEK_SET);
    expect_int(n_iobuf_tell(io), 16);

    i = 1;
    n = snprintf(buf, sizeof(buf), fmt, i);
    x = n_iobuf_read(io, rbuf, n);
    expect_int(x, n);
    expect_int(memcmp(buf, rbuf, n), 0);

    n_iobuf_seek(io, 0, SEEK_SET);
    expect_int(n_iobuf_tell(io), 0);

    i = 0;
    n = snprintf(buf, sizeof(buf), fmt, i);
    x = n_iobuf_read(io, rbuf, n);
    //printf("n = %d, x = %d\n", n, x);
    expect_int(x, n);
    expect_int(memcmp(buf, rbuf, n), 0);
    expect_int(n_iobuf_tell(io), 16);

    i = 1;
    n = snprintf(buf, sizeof(buf), fmt, i);
    x = n_iobuf_read(io, rbuf, n);
    //printf("n = %d, x = %d\n", n, x);
    expect_int(x, n);
    expect_int(memcmp(buf, rbuf, n), 0);
    expect_int(n_iobuf_tell(io), 32);

    n_iobuf_seek(io, 16, SEEK_SET);
    expect_int(n_iobuf_tell(io), 16);

    x = n_iobuf_read(io, rbuf, n);
    //printf("n = %d, x = %d\n", n, x);
    expect_int(x, n);
    expect_int(memcmp(buf, rbuf, n), 0);

    rbuf[x] = '\0';
    nr += x;
    //printf("read %s %d = %d  => %s\n", buf, n, strlen(buf), rbuf);
    n_iobuf_close(io);
}


START_TEST (test_seek)
{
    do_seek(NTEST_TMPPATH("niobuf-seek.zst"));
}
END_TEST


NTEST_RUNNER("niobuf", test_read_write, test_seek);
