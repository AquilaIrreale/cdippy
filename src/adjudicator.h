#ifndef _ADJUDICATOR_H_
#define _ADJUDICATOR_H_

#include <stddef.h>

#include "map.h"

enum kind {
    MOVE,
    SUPPORT,
    CONVOY
};

struct order {
    enum kind kind;      /* Type of order */

    enum territory terr; /* Territory the order
                          * is addressed to
                          */
    enum territory orig; /* For convoy and support orders.
                          * Origin of supported/convoyed move
                          */
    enum territory targ; /* Target territory for moves
                          * (including convoyed) and support
                          * targ == orig in a "support to hold"
                          * order
                          */
    enum coast coast;    /* Target coast
                          */
    /*enum nation nation; * Nationality of unit
                          * to support or convoy
                          */
};

extern struct order orders[34];
extern size_t orders_n;

enum resolution {
    FAILS,
    SUCCEEDS
};

extern enum resolution resolution[34];

enum state {
    UNRESOLVED,
    GUESSING,
    RESOLVED
};

void adjudicate_all();

#endif /* _ADJUDICATOR_H_ */
