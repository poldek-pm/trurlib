/*
  TRURLib
  
  $Id$
*/

/*
   **  stptok() -- public domain by Ray Gardner, modified by Bob Stout
   **
   **   You pass this function a string to parse, a buffer to receive the
   **   "token" that gets scanned, the length of the buffer, and a string of
   **   "break" characters that stop the scan.  It will copy the string into
   **   the buffer up to any of the break characters, or until the buffer is
   **   full, and will always leave the buffer null-terminated.  It will
   **   return a pointer to the first non-breaking character after the one
   **   that stopped the scan.
 */

#include <stdlib.h>
#include <string.h>

#ifdef USE_N_ASSERT
#include "nassert.h"
#else
#include <assert.h>
#define n_assert(expr) assert(expr)
#endif


char *n_str_tok(const char *s, char *tok, size_t toklen, const char *delim)
{
    char *lim;
    const char *p;

    if (!*s)
        return NULL;

    lim = tok + toklen - 1;

    s += strspn(s, delim);
        
    while (*s && tok <= lim) {
        for (p = delim; *p; p++) {
            if (*s == *p) {
                *tok = '\0';
                for (++s, p = delim; *s && *p; ++p) {
                    if (*s == *p) {
                        ++s;
                        p = delim;
                    }
                }
                return (char *) s;
            }
        }

        *tok++ = *s++;
    }
    *tok = '\0';
    return (char *) s;
}


#define TEST_STPTOK 0
#if TEST_STPTOK

#include <string.h>
#include <stdio.h>

main(int argc, char *argv[])
{
    char *ptr, buf[256];

    if (3 > argc) {
        puts("Usage: STPTOK \"string\" \"token_string\"");
        return EXIT_FAILURE;
    } else
        ptr = argv[1];

    do {
        ptr = stptok(ptr, buf, sizeof(buf), argv[2]);

        printf("stptok(\"%s\", \"%s\")\n  buf: \"%s\"\n  "
               "returned: \"%s\"\n", argv[1], argv[2], buf, ptr);

    } while (ptr && *ptr);

    return EXIT_SUCCESS;
}

#endif
