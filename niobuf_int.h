#ifndef TRURL_NIOBUF_INT_H
#define TRURL_NIOBUF_INT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "trurl_internal.h"
#include "ndie.h"
#include "nmalloc.h"
#include "nassert.h"
#include "nstream.h"
#include "niobuf.h"

struct trurl_io {
    char  *name;
    void  *(*new)(int);
    int   (*read)(void *, FILE *, void *, size_t);
    int   (*write)(void *, FILE *, const void *, size_t);
    int   (*flush)(void *, FILE *);
    void  (*reset)(void *);
    int   (*destroy)(void*, FILE *);
};

#define TRURL_IO_MODE_READ  1
#define TRURL_IO_MODE_WRITE 2

#endif /* TRURL_NIOBUF_INT_H */
