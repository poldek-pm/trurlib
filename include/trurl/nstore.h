/* 
  TRURLib
  $Id$
*/

#ifndef TRURL_STORE_H
#define TRURL_STORE_H

#include <stdint.h>
#include <trurl/nstream.h>

int n_store_uint32(tn_stream *st, uint32_t val);
int n_restore_uint32(tn_stream *st, uint32_t *vptr);

#endif
