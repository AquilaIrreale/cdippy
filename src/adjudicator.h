/*
 *  cdippy: a classic Diplomacy adjudicator, in C
 *  Copyright (C) 2018 Simone Cimarelli a.k.a. AquilaIrreale
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _ADJUDICATOR_H_
#define _ADJUDICATOR_H_

#include <config.h>

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
    bool via_convoy;     /* "Via convoy" specifier flag (only for
                          * moves)
                          */
};

extern struct order orders[34];
extern size_t orders_n;

void register_order(enum kind kind,
                    enum territory terr,
                    enum territory orig,
                    enum territory targ,
                    enum coast coast,
                    bool via_convoy);

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
void adjudicator_reset();

bool dislodged(enum territory t);
bool can_retreat(enum territory t1, enum territory t2);

#endif /* _ADJUDICATOR_H_ */
