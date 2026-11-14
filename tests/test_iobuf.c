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
    tn_iobuf *io;
    char rbuf[75];
    int i, x, n;

    io = n_iobuf_open(path, "w");
    for (i = 0; i < nitems; i++) {
        int n = snprintf(buf, sizeof(buf), fmt, i);
        n_iobuf_write(io, buf, n);
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
    //printf("read %s %d = %d  => %s\n", buf, n, strlen(buf), rbuf);
    n_iobuf_close(io);
}


START_TEST (test_seek)
{
    do_seek(NTEST_TMPPATH("niobuf-seek.zst"));
}
END_TEST


/* n_iobuf_gets() keeps fgets semantics: at most size - 1 bytes are
   stored and the terminating nul always stays inside the buffer */
START_TEST (test_gets)
{
    const char *path = NTEST_TMPPATH("niobuf-gets.zst");
    tn_iobuf *io;
    char buf[9];

    io = n_iobuf_open(path, "w");
    fail_if(io == NULL, "cannot create %s", path);
    n_iobuf_write(io, "0123456789\n", 11);
    n_iobuf_write(io, "ab\n", 3);
    n_iobuf_close(io);

    io = n_iobuf_open(path, "r");
    fail_if(io == NULL, "cannot open %s", path);

    buf[8] = 'X';                       /* guard just past the size */
    expect_str(n_iobuf_gets(io, buf, 8), "0123456");
    expect_int(buf[8], 'X');

    expect_str(n_iobuf_gets(io, buf, 8), "789\n");
    expect_int(buf[8], 'X');

    expect_int(n_iobuf_getc(io), 'a');

    expect_str(n_iobuf_gets(io, buf, 8), "b\n");
    expect_str(n_iobuf_gets(io, buf, 8), "");        /* EOF */
    expect_int(n_iobuf_getc(io), EOF);

    n_iobuf_close(io);
}
END_TEST


static tn_iobuf *open_not_zstd(const char *path)
{
    FILE *f = fopen(path, "w");

    fail_if(f == NULL, "cannot create %s", path);
    fputs("this is not a zstd stream\n", f);
    fclose(f);
    return n_iobuf_open(path, "r");
}

START_TEST (test_read_error)
{
    tn_iobuf *io = open_not_zstd(NTEST_TMPPATH("niobuf-notzstd.zst"));
    char buf[16];

    expect_int(n_iobuf_read(io, buf, sizeof(buf)), -1);
    expect_int(n_iobuf_tell(io), 0);
    n_iobuf_close(io);
}
END_TEST

START_TEST (test_read_error_after_seek)
{
    tn_iobuf *io = open_not_zstd(NTEST_TMPPATH("niobuf-notzstd-seek.zst"));
    char buf[16];

    n_iobuf_seek(io, 100, SEEK_SET);    /* skipped to on the next read */
    expect_int(n_iobuf_read(io, buf, sizeof(buf)), -1);
    expect_int(n_iobuf_tell(io), 100);
    n_iobuf_close(io);
}
END_TEST


NTEST_RUNNER("niobuf", test_read_write, test_seek, test_gets, test_read_error,
             test_read_error_after_seek);
