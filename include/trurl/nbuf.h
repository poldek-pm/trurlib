/* 
  TRURLib
  Dynamic buffer
  $Id$
*/

#ifndef TRURL_BUF_H
#define TRURL_BUF_H

#define TN_BUF_CONSTSIZE         (1 << 0)
#define TN_BUF_CONSTDATA         (1 << 1)

#include <stdarg.h>
#include <string.h>

struct trurl_buf {
    unsigned char *data;
    size_t        allocated;
    size_t        size;
    size_t        off;
    unsigned int  flags;
};

typedef struct trurl_buf tn_buf;

tn_buf *n_buf_new(int size);
tn_buf *n_buf_init(tn_buf *buf, void *buffer, int size);
tn_buf *n_buf_clean(tn_buf *buf);
void n_buf_free(tn_buf *buf);

tn_buf *n_buf_ctl(tn_buf *buf, unsigned flags);

int n_buf_write_ex(tn_buf *nbuf, const void *ptr, int size, int zero);
#define n_buf_write(buf, ptr, size)     n_buf_write_ex(buf, ptr, size, 0)
#define n_buf_write_z(buf, ptr, size)   n_buf_write_ex(buf, ptr, size, 1)
#define n_buf_puts(buf, str)            n_buf_write_ex(buf, str, strlen(str), 0)
#define n_buf_puts_z(buf, str)          n_buf_write_ex(buf, str, strlen(str), 1)


int n_buf_seek(tn_buf *buf, int off, int whence);
#define n_buf_tell(buf) n_buf_seek(buf, 0, SEEK_CUR)

/* WARN: limited to 4K */
int n_buf_vprintf(tn_buf *nbuf, const char *fmt, va_list args);
int n_buf_printf(tn_buf *nbuf, const char *fmt, ...);


#include <trurl/n2h.h>
#define n_buf_write_int8(nbuf, v)            \
     do {                                    \
         uint8_t v_ = v;                     \
         n_buf_write(nbuf, &v_, sizeof(v_)); \
     } while(0);

#define n_buf_putc(nbuf, c)      n_buf_write_int8(nbuf, c)
#define n_buf_add_int8(nbuf, v)  n_buf_write_int8(nbuf, v)   

#define n_buf_write_int16(nbuf, v)           \
    do {                                     \
         uint16_t nv = n_hton16(v);          \
         n_buf_write(nbuf, &nv, sizeof(nv)); \
    } while(0);

#define n_buf_add_int16(nbuf, v) n_buf_write_int16(nbuf, v)

#define n_buf_write_int32(nbuf, v)           \
    do {                                     \
         uint32_t nv = n_hton32(v);          \
         n_buf_write(nbuf, &nv, sizeof(nv)); \
    } while(0);

#define n_buf_add_int32(nbuf, v) n_buf_write_int32(nbuf, v)

//int n_buf_size(const tn_buf *buf);
//void *n_buf_ptr(const tn_buf *buf); /* returns buffer data pointer */

static inline int n_buf_size(const tn_buf *buf)
{
    return buf->size;
}

static inline void *n_buf_ptr(const tn_buf *buf)
{
    return buf->data;
}


struct trurl_buf_iterator {
    tn_buf *nbuf;
    size_t  offs;
};

typedef struct trurl_buf_iterator tn_buf_it;
void n_buf_it_init(tn_buf_it *bufi, tn_buf *buf);
void *n_buf_it_get(tn_buf_it *bufi, size_t size);

char *n_buf_it_gets_ext(tn_buf_it *bufi, size_t *len, int endl);
#define n_buf_it_getz(it, len) n_buf_it_gets_ext(it, len, '\0')
#define n_buf_it_gets(it, len) n_buf_it_gets_ext(it, len, '\n')

static inline
int n_buf_it_get_int8(tn_buf_it *nbufi, uint8_t *vp) 
{
    char *p;
    
    p = n_buf_it_get(nbufi, sizeof(*vp));
    if (p == NULL)
        return 0;
    
    *vp = *(uint8_t*)p;
    return 1;
}

static inline
int n_buf_it_get_int16(tn_buf_it *nbufi, uint16_t *vp) 
{
    char *p;
    
    p = n_buf_it_get(nbufi, sizeof(*vp));
    if (p == NULL)
        return 0;
    
    memcpy(vp, p, sizeof(*vp));
    *vp = n_ntoh16(*vp);
    return 1;
}

static inline
int n_buf_it_get_int32(tn_buf_it *nbufi, uint32_t *vp) 
{
    char *p;
    
    p = n_buf_it_get(nbufi, sizeof(*vp));
    if (p == NULL)
        return 0;

    memcpy(vp, p, sizeof(*vp));
    *vp = n_ntoh32(*vp);
    return 1;
}

#include <trurl/nstream.h>

/* sizebits */
#define TN_BUF_STORE_NIL  0  /* (re)store extra byte with bits information */
#define TN_BUF_STORE_8B   1
#define TN_BUF_STORE_16B  2
#define TN_BUF_STORE_32B  3

int n_buf_store(tn_buf *nbuf, tn_stream *st, int sizebits);
int n_buf_store_buf(tn_buf *nbuf, tn_buf *tonbuf, int sizebits);

int n_buf_restore_skip(tn_stream *st, int sizebits);
int n_buf_restore_ex(tn_stream *st, tn_buf **bptr, int sizebits, 
                     int (*process_buf)(tn_buf *, void *), void *arg);

#define n_buf_restore(st, bptr, sizebits) \
                        n_buf_restore_ex(st, bptr, sizebits, NULL, NULL);



/* backward compatibility aliases */
#define n_buf_addata(buf, data, size, zero) n_buf_write_ex(buf, data, size, zero)
#define n_buf_add(buf, data, size)   n_buf_write_ex(buf, data, size, 0)
#define n_buf_addz(buf, data, size)  n_buf_write_ex(buf, data, size, 1)
                   
#define n_buf_addstring(buf, str, zero) n_buf_write_ex(buf, str, strlen(str), zero)
#define n_buf_addstr(buf, str)          n_buf_write_ex(buf, str, strlen(str), 0)
#define n_buf_addstrz(buf, str)         n_buf_write_ex(buf, str, strlen(str), 1)


#endif /* TRURL_BUF_H */
    

        


    


    
