/* $Id$ */

#include "n_hash_int.h"

static
tn_hash *n_hash_put_node(tn_hash *ht, struct hash_bucket *node);

static void n_hash_rehash(tn_hash *ht)
{
    size_t i, newsize, oldsize;
    struct hash_bucket **oldtable;
    unsigned int oldflags;
    
    oldsize  = ht->size;
    oldtable = ht->table;
    oldflags = ht->flags;
    
    newsize = 2 * ht->size + 1;
    
    ht->table = calloc(newsize, sizeof(*ht->table));
    if (ht->table == NULL) {
        ht->table = oldtable;
	return;
    }

    ht->size = newsize;
    ht->items = 0;

    ht->flags = TN_HASH_NOCPKEY;
    for (i = 0; i < oldsize; i++) {
        register struct hash_bucket *node, *next_node;
	if (oldtable[i] == NULL)
            continue;

        for (node = oldtable[i]; node != NULL; node = next_node) {
            next_node = node->next;
            n_hash_put_node(ht, node);
        }
    }

    ht->flags = oldflags;
    free(oldtable);
}

static
tn_hash *n_hash_put_node(tn_hash *ht, struct hash_bucket *node)
{
    struct hash_bucket *ptr;
    unsigned val = ht->hash_fn(node->key);
    

    if (ht->flags & TN_HASH_REHASH && ht->size - ht->items <= ht->size/5) 
        n_hash_rehash(ht);
    
    val %= ht->size;
    ptr =  ht->table[val];
    /*
       ** NULL means this bucket hasn't been used yet.  We'll simply
       ** allocate space for our new bucket and put our data there, with
       ** the table pointing at it.
     */

    if (ptr == NULL) {
	node->next = NULL;
	ht->items++;
	ht->table[val] = node;

	return ht;
    }

    /*
    ** This spot in the table is already in use.  See if the current string
    ** has already been inserted, and if so,  die or replace it
    */
    for (ptr = ht->table[val]; NULL != ptr; ptr = ptr->next) {
	if (strcmp(node->key, ptr->key) == 0) {
            trurl_die("n_hash_put_node: key '%s' already in table\n", node->key);
            return NULL;
        }
    }
    
    node->next = ht->table[val];
    ht->table[val] = node;

    ht->items++;
    return ht;
}


static
tn_hash *n_hash_put(tn_hash *ht, const char *key, const void *data,
                    int replace)
{
    struct hash_bucket *ptr;
    unsigned val = ht->hash_fn(key);
    

    if (ht->flags & TN_HASH_REHASH && ht->size - ht->items <= ht->size/5) 
        n_hash_rehash(ht);
    
    val %= ht->size;
    ptr =  ht->table[val];
    /*
       ** NULL means this bucket hasn't been used yet.  We'll simply
       ** allocate space for our new bucket and put our data there, with
       ** the table pointing at it.
     */

    if (ptr == NULL) {
	if ((ptr = malloc(sizeof(*ptr))) == NULL)
	    return NULL;

        if (ht->flags & TN_HASH_NOCPKEY)
            ptr->key = (char*)key;
        else 
            ptr->key  = strdup(key);
        
	ptr->next = NULL;
	ptr->data = (void *) data;

	ht->items++;
	ht->table[val] = ptr;

	return ht;
    }

    /*
    ** This spot in the table is already in use.  See if the current string
    ** has already been inserted, and if so,  die or replace it
    */
//    printf("INUSE %s, ", key);
    for (ptr = ht->table[val]; NULL != ptr; ptr = ptr->next) {
        //      printf("%s, ", ptr->key);
	if (strcmp(key, ptr->key) == 0) {
	    if (!replace) {
		trurl_die("n_hash_insert: key '%s' already in table\n", key);
		return NULL;
	    }
            
            if (ht->free_fn)
                ht->free_fn(ptr->data);
	    ptr->data = (void *) data;
	    return ht;
	}
    }
//    printf("\n");
    
    
    if ((ptr = malloc(sizeof(*ptr))) == NULL)
	return NULL;

    if (ht->flags & TN_HASH_NOCPKEY)
        ptr->key = (char*)key;
    else 
        ptr->key  = strdup(key);
    ptr->data = (void *) data;
    ptr->next = ht->table[val];
    ht->table[val] = ptr;

    ht->items++;
    return ht;
}

tn_hash *n_hash_insert(tn_hash *ht, const char *key, const void *data)
{
    return n_hash_put(ht, key, data, 0);
}

tn_hash *n_hash_replace(tn_hash *ht, const char *key, const void *data)
{
    return n_hash_put(ht, key, data, 1);
}
