/* 
  TRURLib

  $Id$
*/

#ifndef TRURL_STREAM_H
#define TRURL_STREAM_H

#include <stdint.h>
#include <stdlib.h>
#include <netinet/in.h>

#define hton16(v)  htons(v)
#define hton32(v)  htonl(v)

#define ntoh16(v)  ntohs(v)
#define ntoh32(v)  ntohl(v)

#include "nstream.h"

/* type */
#define TN_STREAM_UNKNOWN -1
#define TN_STREAM_STDIO    1
#define TN_STREAM_GZIO     2

struct trurl_stream_private {
    int   type;
    void  *stream;
    int   fd;

    
    void  *(*open) (const char*, const char *);
    void  *(*dopen) (int, const char *);
    int   (*read)  (void*, void*, size_t);
    char  *(*gets)  (void*, char*, size_t);
    int   (*write) (void*, const void*, size_t);
    int   (*seek)  (void*, long, int);
    long  (*tell)  (void*); 
    int   (*flush) (void*);
    int   (*close)  (void*);
};

typedef struct trurl_stream_private tn_stream;

tn_stream *n_stream_open(const char *path, const char *mode, int type);
tn_stream *n_stream_dopen(int fd, const char *mode, int type);

static inline
int n_stream_read(tn_stream *st, void *buf, size_t size) {
    return st->read(st->stream, buf, size);
}

static inline
int n_stream_read_uint8(tn_stream *st, uint8_t *val)
{
    uint8_t v;
    
    if (st->read(st->stream, &v, sizeof(v)) != sizeof(v))
        return 0;
    
    *val = v;
    return 1;
}

static inline
int n_stream_read_uint16(tn_stream *st, uint16_t *val)
{
    uint16_t v;
    
    if (st->read(st->stream, &v, sizeof(v)) != sizeof(v))
        return 0;
        
    v = ntoh16(v);
    *val = v;
    return 1;
}

static inline
int n_stream_read_uint32(tn_stream *st, uint32_t *val)
{
    uint32_t v;
    
    *val = 0;
    if (st->read(st->stream, &v, sizeof(v)) != sizeof(v))
        return 0;
        
    *val = ntoh32(v);
    return sizeof(v);
}

static inline
int n_stream_write(tn_stream *st, const void *buf, size_t size) {
    return st->write(st->stream, buf, size);
}

static inline
int n_stream_write_uint8(tn_stream *st, uint8_t val)
{
    return st->write(st->stream, &val, 1);
}

static inline
int n_stream_write_uint16(tn_stream *st, uint16_t v)
{
    v = hton16(v);
    return st->write(st->stream, &v, sizeof(v)) == sizeof(v);
}

static inline
int n_stream_write_uint32(tn_stream *st, uint32_t v)
{
    v = hton32(v);
    return st->write(st->stream, &v, sizeof(v)) == sizeof(v);
}



static inline
int n_stream_seek(tn_stream *st, long offset, int whence) {
    return st->seek(st->stream, offset, whence);
}


static inline
long n_stream_tell(tn_stream *st) {
    return st->tell(st->stream);
}


static inline
int n_stream_flush(tn_stream *st) {
    return st->flush(st->stream);
}

static inline
void n_stream_close(tn_stream *st) {
    if (st->stream)
        st->close(st->stream);
    st->stream = NULL;
    free(st);
}

#include <stdarg.h>
int n_stream_vprintf(tn_stream *st, const char *fmt, va_list ap);
int n_stream_printf(tn_stream *st, const char *fmt, ...);
    
#endif
