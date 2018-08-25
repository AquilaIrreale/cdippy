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
    compute_retreats();

    size_t i;
    for (i = 0; i < orders_n; i++) {
        if (resolutions[i] == SUCCEEDS) {
            printf("%zu: SUCCEEDS\n", i);
        } else {
            printf("%zu: FAILS\n", i);
        }
    }

    putchar('\n');

    for (i = 0; i < retreats_n; i++) {
        printf("%s:", terr_name(retreats[i].who));

        size_t j;
        for (j = 0; j < retreats[i].where_n; j++) {
            printf(" %s", terr_name(retreats[i].where[j]));
        }

        putchar('\n');
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
