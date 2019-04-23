#include "test.h"
#include "niobuf.h"

START_TEST (test_read_write)
{
    char buf[75];
    int i, nitems = 10;
    char *fmt = "line%.12d";
    int nw = 0, nr = 0;
    char *tmppath = TMPPATH("niobuf-rw.zst");
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

        memset(rbuf, sizeof(rbuf), 0);

        int x = n_iobuf_read(io, rbuf, n);
        ck_assert(x == n);
        ck_assert(memcmp(buf, rbuf, n) == 0);
        nr += x;
        //printf("read %s %d = %d  => %s\n", buf, n, strlen(buf), rbuf);

    }
    n_iobuf_close(io);
    ck_assert(nw == nr);
    //printf("OK\n");
}
END_TEST

static void seek_and_read(tn_iobuf *io, int nth)
{
    char buf[75], rbuf[75];
    char *fmt = "line%.12d";
    int n, x;


    n_iobuf_seek(io, nth * 16, SEEK_SET);
    ck_assert(n_iobuf_tell(io) == nth * 16);

    n = snprintf(buf, sizeof(buf), fmt, nth);
    x = n_iobuf_read(io, rbuf, n);

    ck_assert(x == n);
    ck_assert(memcmp(buf, rbuf, n) == 0);
}


START_TEST (test_seek)
{
    char buf[75];
    int nitems = 10;
    char *fmt = "line%.12d";
    int nw = 0, nr = 0;
    tn_iobuf *io;
    char rbuf[75];
    int i, x, n, nth;
    char *tmppath = TMPPATH("niobuf-seek.zst");

    io = n_iobuf_open(tmppath, "w");
    for (i = 0; i < 100; i++) {
        int n = snprintf(buf, sizeof(buf), fmt, i);
        int nn = n_iobuf_write(io, buf, n);
        ck_assert(nn == n);
        nw += n;
    }
    n_iobuf_close(io);

    io = n_iobuf_open(tmppath, "r");
    ck_assert(n_iobuf_tell(io) == 0);

    seek_and_read(io, 1);
    ck_assert(n_iobuf_tell(io) == 32);

    seek_and_read(io, 0);
    ck_assert(n_iobuf_tell(io) == 16);

    seek_and_read(io, 1);
    ck_assert(n_iobuf_tell(io) == 32);

    seek_and_read(io, 2);
    ck_assert(n_iobuf_tell(io) == 48);
}
END_TEST


struct test_suite test_suite_niobuf = {
    "n_iobuf",
    {
        { "read/write", test_read_write  },
        { "seek", test_seek  },
        { NULL, NULL }
    }
};
