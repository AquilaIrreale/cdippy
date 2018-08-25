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

#include "cdippy.h"

const char *terr_name(enum territory t);

enum unit get_unit(const char *s);
enum coast get_coast(const char *s);
enum territory get_territory(const char *s);
enum nation get_nation(const char *s);

#define MAX_NEIGH 11

struct terr_info {
    bool occupied;
    enum unit unit;     /* Unit type   */
    enum nation nation; /* Unit nation */
    enum coast coast;   /* For fleets, in applicable territories */

    const enum territory sea_neighs[MAX_NEIGH];
    size_t sea_neighs_n;

    const enum territory land_neighs[MAX_NEIGH];
    size_t land_neighs_n;
};

#define TERR_N 75

extern struct terr_info territories[TERR_N];

void register_unit(enum territory terr,
                   enum coast coast,
                   enum unit unit,
                   enum nation nation);
void clear_unit(enum territory terr);
void clear_all_units();

bool is_single_coast(enum territory t);

bool is_land(enum territory t);
bool is_sea(enum territory t);
bool is_coast(enum territory t);
bool is_inner_land(enum territory t);
bool is_inner_sea(enum territory t);

bool can_reach(enum territory t1,
               enum territory t2,
               enum unit unit,
               enum coast coast);
bool can_support(enum territory t1,
                 enum territory t2,
                 enum unit unit,
                 enum coast coast);

#endif /* _MAP_H_ */
