/*
   Not finished 
 */
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_N_ASSERT
#include "nassert.h"
#else
#include <assert.h>
#define n_assert(expr) assert(expr)
#endif

#ifdef USE_XMALLOCS
#include "xmalloc.h"
#endif


#include "nstring.h"

static char *vconcat(const char *s, va_list * ap);
static char *concat(const char *s,...);

struct string {
    char *s;
    size_t len;
    size_t size;
};


tn_string n_string_construct(char *s)
{
    int len;
    tn_string str;

    len = strlen(s);
    if (len > 0) {
	str.s = strdup(s);
	str.len = len;
	str.size = len + 1;
    }
    return str;
}


static tn_string *n_string_generic_new(char *s)
{
    int len;
    tn_string *str;

    if ((str = malloc(sizeof(*str))) == NULL)
	return NULL;

    len = strlen(s);
    if (len > 0) {
	str->s = s;
	str->len = len;
	str->size = len + 1;
    } else {
	str->s = NULL;
	str->len = 0;
	str->size = 0;

    }

    return str;
}


tn_string *n_string_new(char *s)
{
    return n_string_generic_new(strdup(s));
}


tn_string *n_string_vnew(const char *s,...)
{
    register char *t;
    va_list ap;

    va_start(ap, s);
    t = vconcat(s, ap);
    va_end(ap);

    return n_string_generic_new(t);
}


/* str = '' */
tn_string *n_string_clean(tn_string * str)
{
    if (str->s != NULL)
	free(str->s);
    str->size = str->len = 0;

    return str;
}


void n_string_free(tn_string * str)
{
    n_string_clean(str);
    free(str);
}


static tn_string *n_string_grow(tn_string * str, size_t size)
{

    if (str->len + size >= str->size) {
	char *s;
	if ((s = realloc(str->s, str->size + size)) == NULL)
	    return NULL;

	str->s = s;
	str->size += size;
    }
    return str;
}


const char *n_string_ptr(const tn_string * str)
{
    return str->s;
}


tn_string *n_string_dup(const tn_string * str)
{
    return n_string_new(str->s);
}


/* s += "aaaaa" + "bbbbbbb" + "ccccccc"  */
tn_string *n_string_append_ptr(tn_string * str, const char *src)
{
    register int len;
    register char *p;

    if (str->s == src) {	/* self */
	len = str->len;
/*        p = alloca(len+1); */
	strcpy(p, src);

    } else {
	len = strlen(src);
	p = (char *) src;
    }

    n_string_grow(str, len);
    strncpy(str->s + str->len, p, len);
    str->len += len;

    return str;
}

/* s += s2 */
tn_string *n_string_append(tn_string * str, const tn_string * src)
{
    return n_string_append_ptr(str, n_string_ptr(src));
}

/* s = "a" + s */
tn_string *n_string_prepend_ptr(tn_string * str, const char *src)
{
    int len;

    len = strlen(src);
    n_string_grow(str, len);

    memmove(str->s + str->len, src, len);
    str->len += len;
    return str;
}


/* s = s1 + s2 */
tn_string *n_string_add(const tn_string * str1, const tn_string * str2)
{
    char *s;
    s = concat(n_string_ptr(str1), n_string_ptr(str2), NULL);

    return n_string_generic_new(s);
}


int n_string_cmp(const tn_string * str1, const tn_string * str2)
{
    return strcmp(str1->s, str2->s);
}

int n_string_eq(const tn_string * str1, const tn_string * str2)
{
    return strcmp(str1->s, str2->s) == 0;
}




static char *concat(const char *s,...)
{
    char *t;
    va_list ap;

    va_start(ap, s);
    t = vconcat(s, ap);
    va_end(ap);

    return t;
}


static char *vconcat(const char *s, va_list * ap)
{
    int len;
    char *p, *rstr;
    va_list *tmp_ap;

    len = strlen(s);

    tmp_ap = ap;

    while ((p = va_arg(ap, char *)) != NULL) {	/* calculate length of args */
	len += strlen(p);
    }

    if ((rstr = malloc(len + 1)) == NULL)
	return NULL;

    strcpy(rstr, s);

    ap = tmp_ap;

    while ((p = va_arg(ap, char *)) != NULL) {
	strcat(rstr, p);
    }

    return rstr;
}
