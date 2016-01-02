/* 
  TRURLib
*/
#ifndef __TN_STRING_H
#define __TN_STRING_H

typedef struct string tn_string;

tn_string n_string_construct(char *s);
tn_string *n_string_new(char *s);
tn_string *n_string_newv(const char *s, ...);
void n_string_free(tn_string *str);

/* str = '' */
tn_string *n_string_clean(tn_string *str);
const char *n_string_ptr(const tn_string *str);

/* s = new String(s2) */
tn_string *n_string_dup(const tn_string *str);

/* s += "aaaaa" */
tn_string *n_string_append_ptr(tn_string *str, const char *src);

/* s = "a" + s */
tn_string *n_string_prepend_ptr(tn_string *str, const char *src);


/* s += s2 */
tn_string *n_string_append(tn_string *str, const tn_string *src);


/* s = s1 + s2 */
tn_string *n_string_add(const tn_string *str1, const tn_string *str2);


int n_string_cmp(const tn_string *str1, const tn_string *str2);
int n_string_eq(const tn_string *str1, const tn_string *str2);


#endif
