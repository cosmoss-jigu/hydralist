/*
 * Interface for the No Hot Spot Non-Blocking Skip List
 * operations.
 *
 * Author: Ian Dick, 2013.
 */

#ifndef NOHOTSPOT_OPS_H_
#define NOHOTSPOT_OPS_H_

#include "skiplist.h"

enum sl_optype {
        CONTAINS,
        DELETE,
        INSERT,
        SCAN,
};

typedef enum sl_optype sl_optype_t;

int sl_do_operation(set_t *set, sl_optype_t optype,
                    sl_key_type key, sl_value_type val);

/* these are macros instead of functions to improve performance */
#define sl_contains(a, b) sl_do_operation((a), CONTAINS, (b), NULL);
#define sl_delete(a, b) sl_do_operation((a), DELETE, (b), NULL);
#define sl_insert(a, b, c) sl_do_operation((a), INSERT, (b), (c));
// Note that the range must keep valid before this function returns
#define sl_scan(a, start_key, range) sl_do_operation((a), SCAN, (start_key), (void *)&(range))

#endif /* NOHOTSPOT_OPS_H_ */
