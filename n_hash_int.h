/*
  TRURLib, hash table
  Copyright (C) 1999, 2000 Pawel A. Gajda (mis@k2.net.pl)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  
Module is based on:
**
** public domain code by Jerry Coffin.
**
** Tested with Visual C 1.0 and Borland C 3.1.
** Compiles without warnings, and seems like it should be pretty
** portable.
*/

#include <string.h>
#include <stdlib.h>


#include "trurl_internal.h"
#include "nhash.h"
#include "nmalloc.h"

#define USE_HASHSTRING_GLIBC_DB 0
#define USE_HASHSTRING_CDB 1

/*
   ** A hash table consists of an array of these buckets.  Each bucket
   ** holds a copy of the key, a pointer to the data associated with the
   ** key, and a pointer to the next bucket that collided with this one,
   ** if there was one.
 */

struct hash_bucket {
    void  *data;
    char  *key;
    struct hash_bucket *next;
    char  _buf[0];
};


/*
   ** This is what you actually declare an instance of to create a table.
   ** You then call 'construct_table' with the address of this structure,
   ** and a guess at the size of the table.  Note that more nodes than this
   ** can be inserted in the table, but performance degrades as this
   ** happens.  Performance should still be quite adequate until 2 or 3
   ** times as many nodes have been inserted as the table was created with.
 */

struct trurl_hash_table {
    uint16_t    _refcnt;
    uint16_t    flags;

    size_t size;
    struct hash_bucket **table;
    size_t items;
    
    void          (*free_fn) (void *);
    unsigned int  (*hash_fn) (const char*);
};

int n_hash_dohash(const tn_hash *ht, const char *s, int *slen);

#define n_hash_nextslotv(key, klen) (klen + (int)key[klen - 1])
