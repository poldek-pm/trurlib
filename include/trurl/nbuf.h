/* 
  TRURLib
  Dynamic buffer
  $Id$
*/

#ifndef TRURL_BUF_H
#define TRURL_BUF_H

typedef struct trurl_buf tn_buf;

tn_buf *n_buf_new(int size);
tn_buf *n_buf_clean(tn_buf *buf);

void n_buf_free(tn_buf *buf);

void *n_buf_add(tn_buf *buf, const void *data, int size);

char *n_buf_addstring(tn_buf *buf, const char *str, int with_zero);

#define n_buf_addstr(buf, str)  n_buf_addstring(buf, str, 0)
#define n_buf_addstrz(buf, str) n_buf_addstring(buf, str, 1)

int n_buf_size(const tn_buf *buf);
void *n_buf_ptr(const tn_buf *buf); /* returns buffer data pointer */

#endif /* TRURL_BUF_H */
    

        


    


    
