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

#ifndef _MAP_H_
#define _MAP_H_

#include <config.h>

#include <stddef.h>
#include <stdbool.h>

#include <cdippy.h>

const char *terr_name(enum cd_terr t);

#define MAX_NEIGH 11

struct terr_info {
    bool occupied;
    enum cd_unit unit;     /* Unit type   */
    enum cd_nation nation; /* Unit nation */
    enum cd_coast coast;   /* For fleets, in applicable territories */

    const enum cd_terr sea_neighs[MAX_NEIGH];
    size_t sea_neighs_n;

    const enum cd_terr land_neighs[MAX_NEIGH];
    size_t land_neighs_n;
};

#define TERR_N 75

extern struct terr_info territories[TERR_N];

void register_unit(enum cd_terr terr,
                   enum cd_coast coast,
                   enum cd_unit unit,
                   enum cd_nation nation);
void clear_unit(enum cd_terr terr);
void clear_all_units();

bool is_single_coast(enum cd_terr t);

bool is_land(enum cd_terr t);
bool is_sea(enum cd_terr t);
bool is_coast(enum cd_terr t);
bool is_inner_land(enum cd_terr t);
bool is_inner_sea(enum cd_terr t);

bool can_reach(enum cd_terr t1,
               enum cd_terr t2,
               enum cd_unit unit,
               enum cd_coast coast);
bool can_support(enum cd_terr t1,
                 enum cd_terr t2,
                 enum cd_unit unit,
                 enum cd_coast coast);

#endif /* _MAP_H_ */
