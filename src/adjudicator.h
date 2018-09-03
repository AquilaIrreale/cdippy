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

#include <cdippy.h>
#include "map.h"

enum kind {
    MOVE,
    SUPPORT,
    CONVOY
};

struct order {
    enum kind kind;      /* Type of order */

    enum cd_terr terr; /* Territory the order
                          * is addressed to
                          */
    enum cd_terr orig; /* For convoy and support orders.
                          * Origin of supported/convoyed move
                          */
    enum cd_terr targ; /* Target territory for moves
                          * (including convoyed) and support
                          * targ == orig in a "support to hold"
                          * order
                          */
    enum cd_coast coast;    /* Target coast
                          */
    bool via_convoy;     /* "Via convoy" specifier flag (only for
                          * moves)
                          */
};

extern struct order orders[MAX_ORDERS];
extern size_t orders_n;

void cd_register_order(enum kind kind,
                       enum cd_terr terr,
                       enum cd_terr orig,
                       enum cd_terr targ,
                       enum cd_coast coast,
                       bool via_convoy);

enum state {
    UNRESOLVED,
    GUESSING,
    RESOLVED
};

void cd_adjudicate_all();
void cd_compute_retreats();
void cd_adjudicator_reset();

bool cd_dislodged(enum cd_terr t);
bool cd_can_retreat(enum cd_terr t1, enum cd_terr t2);

#endif /* _ADJUDICATOR_H_ */
