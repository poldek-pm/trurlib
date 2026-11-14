#include <fcntl.h>
#include <unistd.h>

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

/* zstd_read() prints each decompression error it meets */
static int count_zstd_errors(const char *path)
{
    char buf[4096], *p = buf;
    int fd = open(path, O_RDONLY), n, count = 0;

    fail_if(fd < 0, "cannot open %s", path);
    n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    buf[n > 0 ? n : 0] = '\0';

    while ((p = strstr(p, "ZSTD_decompressStream:")) != NULL) {
        count++;
        p++;
    }
    return count;
}

START_TEST (test_read_after_error)
{
    tn_iobuf *io = open_not_zstd(NTEST_TMPPATH("niobuf-notzstd-again.zst"));
    const char *errpath = NTEST_TMPPATH("niobuf-notzstd-again.err");
    char buf[16];
    int fd, err;

    fflush(stderr);
    err = dup(2);
    fd = open(errpath, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    fail_if(err < 0 || fd < 0, "cannot redirect stderr to %s", errpath);
    dup2(fd, 2);
    close(fd);

    expect_int(n_iobuf_read(io, buf, sizeof(buf)), -1);
    expect_int(n_iobuf_read(io, buf, sizeof(buf)), -1);

    fflush(stderr);
    dup2(err, 2);
    close(err);
    expect_int(count_zstd_errors(errpath), 1);  /* not decompressed again */
    n_iobuf_close(io);
}
END_TEST

START_TEST (test_seek_back_after_error)
{
    const char *path = NTEST_TMPPATH("niobuf-garbage-after.zst");
    tn_iobuf *io;
    char buf[32];
    FILE *f;

    io = n_iobuf_open(path, "w");
    fail_if(io == NULL, "cannot create %s", path);
    n_iobuf_write(io, "0123456789abcdef", 16);
    n_iobuf_close(io);

    f = fopen(path, "a");
    fail_if(f == NULL, "cannot append to %s", path);
    fputs("this is not a zstd stream\n", f);
    fclose(f);

    io = n_iobuf_open(path, "r");
    expect_int(n_iobuf_read(io, buf, 4), 4);
    expect_int(n_iobuf_read(io, buf, sizeof(buf)), -1);  /* past the frame */
    n_iobuf_seek(io, 0, SEEK_SET);      /* starts the stream again */
    expect_int(n_iobuf_read(io, buf, 16), 16);
    expect_int(memcmp(buf, "0123456789abcdef", 16), 0);
    n_iobuf_close(io);
}
END_TEST

START_TEST (test_seek_past_end)
{
    const char *path = NTEST_TMPPATH("niobuf-seek-end.zst");
    tn_iobuf *io;
    char buf[16];

    io = n_iobuf_open(path, "w");
    fail_if(io == NULL, "cannot create %s", path);
    n_iobuf_write(io, "0123456789abcdef", 16);
    n_iobuf_close(io);

    io = n_iobuf_open(path, "r");
    n_iobuf_seek(io, 100, SEEK_SET);
    expect_int(n_iobuf_read(io, buf, sizeof(buf)), 0);
    expect_int(n_iobuf_tell(io), 100);  /* the offset asked for, as ftell() */

    n_iobuf_seek(io, -90, SEEK_CUR);
    expect_int(n_iobuf_read(io, buf, sizeof(buf)), 6);
    expect_int(memcmp(buf, "abcdef", 6), 0);
    n_iobuf_close(io);
}
END_TEST


NTEST_RUNNER("niobuf", test_read_write, test_seek, test_gets, test_read_error,
             test_read_error_after_seek, test_read_after_error,
             test_seek_back_after_error, test_seek_past_end);
