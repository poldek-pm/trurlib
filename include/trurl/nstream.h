/*
  TRURLib

  $Id$
*/

#ifndef TRURL_STREAM_H
#define TRURL_STREAM_H

#include <stdlib.h>

#include <trurl/n2h.h>

/* type */
#define TN_STREAM_UNKNOWN -1
#define TN_STREAM_STDIO    1
#define TN_STREAM_GZIO     2
#define TN_STREAM_ZSTDIO   3

struct trurl_stream_private {
    int   type;
    void  *stream;
    int   fd;


    void  *(*st_open) (const char*, const char *);
    void  *(*st_dopen)(int, const char *);
    int   (*st_read)  (void*, void*, size_t);
    char  *(*st_gets) (void*, char*, size_t);
    int   (*st_getc)  (void*);
    int   (*st_ungetc)  (int, void *);
    int   (*st_write) (void*, const void*, size_t);
    int   (*st_seek)  (void*, long, int);
    long  (*st_tell)  (void*);
    int   (*st_flush) (void*);
    int   (*st_close)  (void*);
    int   (*_write_hook)(const void*, size_t, void *);
    void   *_write_hook_arg;
};

typedef struct trurl_stream_private tn_stream;

tn_stream *n_stream_open(const char *path, const char *mode, int type);
tn_stream *n_stream_dopen(int fd, const char *mode, int type);

static inline
int n_stream_fdno(tn_stream *st)
{
    return st->fd;
}

static inline
int n_stream_set_write_hook(tn_stream *st,
                            int (*write_hook)(const void*, size_t, void *),
                            void *write_hook_arg)
{
    st->_write_hook = write_hook;
    st->_write_hook_arg = write_hook_arg;
    return 1;
}

/* returns the number of bytes read */
int n_stream_gets(tn_stream *st, char *buf, size_t size);
int n_stream_getline(tn_stream *st, char **bufptr, size_t size);

static inline
int n_stream_read(tn_stream *st, void *buf, size_t size) {
    return st->st_read(st->stream, buf, size);
}

static inline
int n_stream_read_uint8(tn_stream *st, uint8_t *val)
{
    uint8_t v;

    if (st->st_read(st->stream, &v, sizeof(v)) != sizeof(v))
        return 0;

    *val = v;
    return 1;
}

static inline
int n_stream_read_uint16(tn_stream *st, uint16_t *val)
{
    uint16_t v;

    if (st->st_read(st->stream, &v, sizeof(v)) != sizeof(v))
        return 0;

    v = n_ntoh16(v);
    *val = v;
    return 1;
}

static inline
int n_stream_read_uint32(tn_stream *st, uint32_t *val)
{
    uint32_t v;

    *val = 0;
    if (st->st_read(st->stream, &v, sizeof(v)) != sizeof(v))
        return 0;

    *val = n_ntoh32(v);
    return sizeof(v);
}

static inline
int n_stream_write(tn_stream *st, const void *buf, size_t size) {
    if (st->_write_hook)
        if (!st->_write_hook(buf, size, st->_write_hook_arg))
            return size;        /* fake write */

    return st->st_write(st->stream, buf, size);
}

static inline
int n_stream_write_uint8(tn_stream *st, uint8_t val)
{
    return n_stream_write(st, &val, 1);
}

static inline
int n_stream_write_uint16(tn_stream *st, uint16_t v)
{
    v = n_hton16(v);
    return n_stream_write(st, &v, sizeof(v)) == sizeof(v);
}

static inline
int n_stream_write_uint32(tn_stream *st, uint32_t v)
{
    v = n_hton32(v);
    return n_stream_write(st, &v, sizeof(v)) == sizeof(v);
}

static inline
int n_stream_seek(tn_stream *st, long offset, int whence) {
    return st->st_seek(st->stream, offset, whence);
}

static inline
long n_stream_tell(tn_stream *st) {
    return st->st_tell(st->stream);
}

static inline
int n_stream_flush(tn_stream *st) {
    return st->st_flush(st->stream);
}

static inline
void n_stream_close(tn_stream *st) {
    if (st->stream)
        st->st_close(st->stream);
    st->stream = NULL;
    free(st);
}

#include <stdarg.h>
int n_stream_vprintf(tn_stream *st, const char *fmt, va_list ap);
int n_stream_printf(tn_stream *st, const char *fmt, ...);

#endif
