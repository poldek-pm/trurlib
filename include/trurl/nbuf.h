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

typedef struct trurl_buf tn_buf;

tn_buf *n_buf_new(int size);
tn_buf *n_buf_init(tn_buf *buf, void *buffer, int size);
tn_buf *n_buf_clean(tn_buf *buf);
void n_buf_free(tn_buf *buf);

tn_buf *n_buf_ctl(tn_buf *buf, unsigned flags);

int n_buf_addata(tn_buf *buf, const void *data, int size, int with_zero);

#define n_buf_add(buf, data, size) n_buf_addata(buf, data, size, 0)
#define n_buf_addz(buf, data, size) n_buf_addata(buf, data, size, 1)

int n_buf_addstring(tn_buf *buf, const char *str, int with_zero);
#define n_buf_addstr(buf, str)  n_buf_addstring(buf, str, 0)
#define n_buf_addstrz(buf, str) n_buf_addstring(buf, str, 1)

/* WARN: limited to 4K */
int n_buf_vprintf(tn_buf *nbuf, const char *fmt, va_list args);
int n_buf_printf(tn_buf *nbuf, const char *fmt, ...);



int n_buf_size(const tn_buf *buf);
void *n_buf_ptr(const tn_buf *buf); /* returns buffer data pointer */


struct trurl_buf_iterator {
    tn_buf *nbuf;
    size_t  offs;
};

typedef struct trurl_buf_iterator tn_buf_it;

void n_buf_it_init(tn_buf_it *bufi, tn_buf *buf);
void *n_buf_it_get(tn_buf_it *bufi, size_t size);

#include <trurl/nstream.h>

/* sizebits */
#define TN_BUF_STORE_NIL  0  /* (re)store extra byte with bits information */
#define TN_BUF_STORE_8B   1
#define TN_BUF_STORE_16B  2
#define TN_BUF_STORE_32B  3

int n_buf_store(tn_buf *nbuf, tn_stream *st, int sizebits);
int n_buf_restore_skip(tn_stream *st, int sizebits);
int n_buf_restore_ex(tn_stream *st, tn_buf **bptr, int sizebits, 
                     int (*process_buf)(tn_buf *, void *), void *arg);

#define n_buf_restore(st, bptr, sizebits) \
                        n_buf_restore_ex(st, bptr, sizebits, NULL, NULL);

#endif /* TRURL_BUF_H */
    

        


    


    
