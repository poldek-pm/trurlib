/* $Id$ */

#include <limits.h>
#include <netinet/in.h>

#include "nstore.h"
#include "nstream.h"
#include "nassert.h"

#define hton16(v)  htons(v)
#define hton32(v)  htonl(v)

#define ntoh16(v)  ntohs(v)
#define ntoh32(v)  ntohl(v)

#define N_STORE_8B   0
#define N_STORE_16B  1
#define N_STORE_32B  2

int n_store_uint32(tn_stream *st, uint32_t val) 
{
    uint8_t  val8;
    uint16_t val16;
    uint32_t val32;
    uint8_t  flag;
    
    int size_of_size = 0;
    void *size_ptr = NULL;
    
    val32 = val;
    
    if (val32 < UINT8_MAX) {
        flag = N_STORE_8B;
        val8 = val32;
        size_of_size = sizeof(val8);
        size_ptr = &val8;
        
    } else if (val32 < UINT16_MAX) {
        flag = N_STORE_16B;
        val16 = val32;
        val16 = hton16(val16);
        
        size_of_size = sizeof(val16);
        size_ptr = &val16;
        
    } else {
        flag = N_STORE_32B;
        val32 = hton32(val32);
        
        size_of_size = sizeof(val32);
        size_ptr = &val32;
    }

    
    n_stream_write(st, &flag, sizeof(flag));
    return n_stream_write(st, size_ptr, size_of_size) == size_of_size;
}

int n_restore_uint32(tn_stream *st, uint32_t *vptr) 
{
    uint32_t       val = 0;
    uint8_t        flag;
    int            rc = 0;


    *vptr = 0;

    if (n_stream_read(st, &flag, sizeof(flag)) != sizeof(flag))
        return 0;
    
    switch (flag) {
        case N_STORE_8B: {
            uint8_t val8;
            if (n_stream_read(st, &val8, sizeof(val8)) != sizeof(val8))
                return 0;
            val = val8;
        }
            break;
            
        case N_STORE_16B: {
            uint16_t val16;
            if (n_stream_read(st, &val16, sizeof(val16)) != sizeof(val16))
                return 0;
            val16 = ntoh16(val16);
            val = val16;
        }
            break;
            
        case N_STORE_32B: {
            uint32_t val32;
            if (n_stream_read(st, &val32, sizeof(val32)) != sizeof(val32))
                return 0;
            val32 = ntoh32(val32);
            val = val32;
        }
            break;

        default:
            n_assert(0);
    }

    *vptr = val;
    return rc;
}

