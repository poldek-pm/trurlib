#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "nassert.h"

char *n_str_strip_ws(char *str)
{
    char *p;

    if (str == NULL)
        return str;

    p = str;
    while(isspace(*p))
        p++;

    str = p;
    p = strchr(str, '\0');
    n_assert(p);
    p--;

    while (p != str && isspace(*p)) {
        *p = '\0';
        p--;
    }

    return str;
}

static
int n_str_vin(const char *s, va_list ap)
{
    char *p;
    va_list save_ap;

    __va_copy(save_ap, ap);

    while ((p = va_arg(ap, char *)) != NULL) {
        if (strcmp(p, s) == 0)
            return 1;
    }

    __va_copy(ap, save_ap);

    return 0;
}


int n_str_in(const char *s,...)
{
    va_list ap;

    va_start(ap, s);
    int in = n_str_vin(s, ap);
    va_end(ap);

    return in;
}
