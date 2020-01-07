/*
  TRURLib

  $Id$
*/

#ifndef TRURL_IOBUF_H
#define TRURL_IOBUF_H

#include <stdlib.h>

struct trurl_iobuf;
typedef struct trurl_iobuf tn_iobuf;

tn_iobuf *n_iobuf_open(const char *path, const char *mode);
tn_iobuf *n_iobuf_dopen(int fd, const char *mode);
int n_iobuf_read(tn_iobuf *iobuf, void *buf, size_t size);
int n_iobuf_write(tn_iobuf *iobuf, const void *buf, size_t size);
int n_iobuf_flush(tn_iobuf *iobuf);
int n_iobuf_close(tn_iobuf *iobuf);

int n_iobuf_seek(tn_iobuf *iobuf, long offset, int whence);
long n_iobuf_tell(tn_iobuf *iobuf);

char *n_iobuf_gets(tn_iobuf *iobuf, char *dest, size_t size);
int n_iobuf_getc(tn_iobuf *iobuf);



#endif
