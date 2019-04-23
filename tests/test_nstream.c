#include "test.h"
#include "nstream.h"
#include "nstore.h"

void do_test_read_write(const char *tmppath)
{
    char buf[75];
    int i, nitems = 10;
    char *fmt = "line%.12d";
    int nw = 0, nr = 0;

    tn_stream *io = n_stream_open(tmppath, "w", TN_STREAM_UNKNOWN);
    for (i = 0; i < nitems; i++) {
        int n = snprintf(buf, sizeof(buf), fmt, i);
        nw += n_stream_write(io, buf, n);
    }
    n_stream_close(io);

    io = n_stream_open(tmppath, "r", TN_STREAM_UNKNOWN);
    for (i = 0; i < nitems; i++) {
        int n = snprintf(buf, sizeof(buf), fmt, i);
        char rbuf[75];

        memset(rbuf, sizeof(rbuf), 0);

        int x = n_stream_read(io, rbuf, n);
        ck_assert(x == n);
        ck_assert(memcmp(buf, rbuf, n) == 0);
        nr += x;
        //printf("read %s %d = %d  => %s\n", buf, n, strlen(buf), rbuf);

    }
    n_stream_close(io);
    ck_assert(nw == nr);
    //printf("OK\n");
}

START_TEST(test_read_write)
{
    do_test_read_write(TMPPATH("nstream-rw.txt"));
    do_test_read_write(TMPPATH("nstream-rw.gz"));
    do_test_read_write(TMPPATH("nstream-rw.zst"));
}
END_TEST

static void seek_and_read(tn_stream *io, int nth)
{
    char buf[75], rbuf[75];
    char *fmt = "line%.12d";
    int n, x;


    n_stream_seek(io, nth * 16, SEEK_SET);
    ck_assert(n_stream_tell(io) == nth * 16);

    n = snprintf(buf, sizeof(buf), fmt, nth);
    x = n_stream_read(io, rbuf, n);

    ck_assert(x == n);
    ck_assert(memcmp(buf, rbuf, n) == 0);
}

void do_test_seek(const char *tmppath)
{
    char buf[75];
    int nitems = 10;
    char *fmt = "line%.12d";
    int nw = 0, nr = 0;
    tn_stream *io;
    char rbuf[75];
    int i, x, n, nth;

    io = n_stream_open(tmppath, "w", TN_STREAM_UNKNOWN);
    for (i = 0; i < 100; i++) {
        int n = snprintf(buf, sizeof(buf), fmt, i);
        int nn = n_stream_write(io, buf, n);
        ck_assert(nn == n);
        nw += n;
    }
    n_stream_close(io);

    io = n_stream_open(tmppath, "r", TN_STREAM_UNKNOWN);
    ck_assert(n_stream_tell(io) == 0);

    seek_and_read(io, 1);
    ck_assert(n_stream_tell(io) == 32);

    seek_and_read(io, 0);
    ck_assert(n_stream_tell(io) == 16);

    seek_and_read(io, 1);
    ck_assert(n_stream_tell(io) == 32);

    seek_and_read(io, 2);
    ck_assert(n_stream_tell(io) == 48);
}

START_TEST(test_seek) {
    do_test_seek(TMPPATH("nstream-seek.txt"));
    do_test_seek(TMPPATH("nstream-seek.gz"));
    do_test_seek(TMPPATH("nstream-seek.zst"));
}
END_TEST


void test_getline(const char *name)
{
    tn_stream *st;
    char *buf = n_malloc(16);
    int n;

    st = n_stream_open(name, "r", TN_STREAM_UNKNOWN);
    n = n_stream_getline(st, &buf, 16);
    printf("getline = %d (%s)\n", n, buf);
    n_assert(strlen(buf) == n);
    n_stream_close(st);
}


struct test_suite test_suite_nstream = {
    "n_stream",
    {
        { "read/write", test_read_write  },
        { "seek", test_seek  },
        { NULL, NULL }
    }
};
