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

#include <config.h>

#include <stdio.h>

#include "parser.h"

#include "map.h"
#include "adjudicator.h"

void execute()
{
    adjudicate_all();

    size_t i;
    for (i = 0; i < orders_n; i++) {
        if (resolution[i] == SUCCEEDS) {
            printf("%llu: SUCCEEDS\n", (long long unsigned)i);
        } else {
            printf("%llu: FAILS\n", (long long unsigned)i);
        }
    }

    putchar('\n');

    enum territory t1;
    for (t1 = 0; t1 < TERR_N; t1++) {
        if (dislodged(t1)) {
            printf("%s:", terr_name(t1));

            const enum territory *neighs = territories[t1].unit == ARMY
                                         ? territories[t1].land_neighs
                                         : territories[t1].sea_neighs;

            size_t neighs_n = territories[t1].unit == ARMY
                            ? territories[t1].land_neighs_n
                            : territories[t1].sea_neighs_n;

            for (i = 0; i < neighs_n; i++) {
                enum territory t2 = neighs[i];

                if (can_retreat(t1, t2)) {
                    printf(" %s", terr_name(t2));
                }
            }

            putchar('\n');
        }
    }

    putchar('\n');

    orders_n = 0;
}

int main(int argc, char *argv[])
{
    if (argc > 1) {
        fputs("cdippy does not take any command-line arguments", stderr);
        return 1;
    }

    clear_all_units();

    while (yyparse());

    return 0;
}
