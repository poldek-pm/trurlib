/* $Id$ */

#include "n_hash_int.h"

void *n_hash_remove(tn_hash *ht, const char *key)
{
    unsigned val = ht->hash_fn(key) % ht->size;
    void *data;
    struct hash_bucket *ptr, *last = NULL;

    if (ht->table[val] == NULL)
        return 0;

    /*
    ** Traverse the list, keeping track of the previous node in the list.
    ** When we find the node to delete, we set the previous node's next
    ** pointer to point to the node after ourself instead.  We then delete
    ** the key from the present node, and return a pointer to the data it
    ** contains.
    */

    for (last = NULL, ptr = ht->table[val]; ptr != NULL;
         last = ptr, ptr = ptr->next) {

        if (strcmp(key, ptr->key) == 0) {

            if (last != NULL) {
                data = ptr->data;

                last->next = ptr->next;
                if (!(ht->flags & TN_HASH_NOCPKEY))
                    free(ptr->key);

                free(ptr);
                ht->items--;

                return data;
            }
            /* If 'last' still equals NULL, it means that we need to
            ** delete the first node in the list. This simply consists
            ** of putting our own 'next' pointer in the array holding
            ** the head of the list.  We then dispose of the current
            ** node as above.
            */
            else {
                ht->table[val] = ptr->next;
                data = ptr->data;

                if (!(ht->flags & TN_HASH_NOCPKEY))
                    free(ptr->key);
                free(ptr);
                ht->items--;

                return data;
            }
        }
    }

    /*
    ** If we get here, it means we didn't find the item in the table.
    ** Signal this by returning NULL.
    */
    return NULL;
}
