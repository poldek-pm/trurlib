/* 
  TRURLib

  $Id$
*/
#ifndef TRURL_N2H_H
#define TRURL_N2H_H

#include <stdint.h>
#include <netinet/in.h>

#define n_hton16(v)  htons(v)
#define n_hton32(v)  htonl(v)

#define n_ntoh16(v)  ntohs(v)
#define n_ntoh32(v)  ntohl(v)

#endif
