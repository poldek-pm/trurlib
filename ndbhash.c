/* 
  TRURLib
  Copyright (C) 1999 Pawel Gajda (mis@k2.net.pl)
 
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.
 
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.
 
  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/

/*
  $Id$
*/
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if defined __GNU_LIBRARY__ && defined __GLIBC_MINOR__ && \
            __GNU_LIBRARY__ == 6  && __GLIBC_MINOR__ > 0 
# include <db_185.h>
#else 
# include <db.h>
#endif


#ifdef USE_N_ASSERT
#include "nassert.h"
#else
#include <assert.h>
#define n_assert(expr) assert(expr)
#endif


#ifdef USE_XMALLOCS
#include "xmalloc.h"
#endif

#include "ndbhash.h"

struct trurl_dbhash {
    DB *db;
    struct ndbhash_data dbt;
};


tn_dbhash *n_dbhash_new_ex(const char *file, int flags, int mode, void *hinfo)
/*                            HASHINFO *hinfo) */
{
    tn_dbhash *ht = NULL;

    if ((ht = malloc(sizeof(*ht))) == NULL)
	return NULL;

    ht->db = dbopen(file, flags, mode, DB_HASH, NULL);

    if (ht->db == NULL) {
	free(ht);
	return NULL;
    }
    return ht;
}


static tn_dbhash *n_dbhash_put(tn_dbhash *ht,
			       const char *key,
			       const void *data, size_t data_size,
			       u_int flags)
{
    int rc;

    DBT dbt_key;
    DBT dbt_data;

    dbt_key.data = (void *) key;
    dbt_key.size = strlen(key) + 1;

    dbt_data.data = (void *) data;
    dbt_data.size = data_size;


    rc = ht->db->put(ht->db, &dbt_key, &dbt_data, flags);

    if (rc != 0)
	ht = NULL;

    return ht;
}



tn_dbhash *n_dbhash_insert(tn_dbhash *ht, const char *key,
			   const void *data, size_t data_size)
{
    return n_dbhash_put(ht, key, data, data_size, R_NOOVERWRITE);
}


tn_dbhash *n_dbhash_replace(tn_dbhash *ht, const char *key,
			    const void *data, size_t data_size)
{
    return n_dbhash_put(ht, key, data, data_size, 0);
}


const struct ndbhash_data *n_dbhash_get(const tn_dbhash *ht, const char *key)
{
    register tn_dbhash *h = (tn_dbhash *) ht;

    DBT dbt_key;
    DBT dbt_data;


    dbt_key.data = (void *) key;
    dbt_key.size = strlen(key) + 1;

    if (h->db->get(h->db, &dbt_key, &dbt_data, 0) != 0)
	return NULL;

    h->dbt.data = dbt_data.data;
    h->dbt.size = dbt_data.size;

    return &ht->dbt;
}


const void *n_dbhash_get_data(const tn_dbhash *ht, const char *key)
{
    const struct ndbhash_data *tmp;

    if ((tmp = n_dbhash_get(ht, key)) != NULL)
	return tmp->data;

    return NULL;
}


int n_dbhash_remove(tn_dbhash *ht, const char *key)
{
    DBT dbt_key;

    dbt_key.data = (void *) key;
    dbt_key.size = strlen(key) + 1;

    return ht->db->del(ht->db, &dbt_key, 0) == 0;
}


void n_dbhash_free(tn_dbhash *ht)
{
    ht->db->close(ht->db);
    free(ht);
}

/*
   int n_dbhash_size(const tn_dbhash *ht) 
   {
   register tn_dbhash *h = (tn_dbhash*)ht;
   DBT dbt_key, dbt_data;
   int n = 0;


   if(h->db->seq(h->db, &dbt_key, &dbt_data, R_FIRST) != 0)
   return -1;

   n++;

   while(h->db->seq(h->db, &dbt_key, &dbt_data, R_NEXT) == 0)
   n++;

   h->items = n;

   return n;
   }
 */



int n_dbhash_map(const tn_dbhash *ht,
		 void (*map_fn) (const void *, const void *, size_t))
{
    int n = 0;
    DBT dbt_key, dbt_data;


    if (ht->db->seq(ht->db, &dbt_key, &dbt_data, R_FIRST) != 0)
	return 0;

    map_fn(dbt_key.data, dbt_data.data, dbt_data.size);
    n++;


    while (ht->db->seq(ht->db, &dbt_key, &dbt_data, R_NEXT) == 0) {
	map_fn(dbt_key.data, dbt_data.data, dbt_data.size);
	n++;
    }

    return n;
}


int n_dbhash_map_arg(tn_dbhash *ht,
                     void (*map_fn) (const char *, const void *, size_t, void *),
		     void *arg)
{
    int n = 0;
    DBT dbt_key, dbt_data;

    if (ht->db->seq(ht->db, &dbt_key, &dbt_data, R_FIRST) != 0)
        return 0;

    n++;
    map_fn(dbt_key.data, dbt_data.data, dbt_data.size, arg);

    while (ht->db->seq(ht->db, &dbt_key, &dbt_data, R_NEXT) == 0) {
        map_fn(dbt_key.data, dbt_data.data, dbt_data.size, arg);
        n++;
    }
    return n;
}
