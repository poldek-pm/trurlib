/* 
  TRURLib

  $Id$
*/

#ifndef TRURL__INTERNAL_H
#define TRURL__INTERNAL_H

extern void trurl_die(const char *fmt,...);
extern int trurl_default_cmpf(const void *a, const void *b);


#if ENABLE_TRACE
# define DBGF(fmt, args...)  fprintf(stdout, "%-18s: " fmt, __FUNCTION__ , ## args)
# define DBG(fmt, args...)   fprintf(stdout, fmt, ## args)
#else 
# define DBGF(fmt, args...)  ((void) 0)
# define DBG(fmt, args...)    ((void) 0)
#endif

#define DBGMSG_F DBGF
#define DBGMSG   DBG

#define DBGF_NULL(fmt, args...) ((void) 0)
#define DBGF_F(fmt, args...) fprintf(stdout, "%-18s: " fmt, __FUNCTION__ , ## args)

#define dbgf(fmt, args...)  fprintf(stdout, "%-18s: " fmt, __FUNCTION__ , ## args)
#define dbgf_(fmt, args...) ((void) 0)

#endif /* TRURL__INTERNAL_H */
