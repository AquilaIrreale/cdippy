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
            printf("%llu:", (long long unsigned)t1);

            const enum territory *neighs = territories[t1].unit == ARMY
                                         ? territories[t1].land_neighs
                                         : territories[t1].sea_neighs;

            size_t neighs_n = territories[t1].unit == ARMY
                            ? territories[t1].land_neighs_n
                            : territories[t1].sea_neighs_n;

            for (i = 0; i < neighs_n; i++) {
                enum territory t2 = neighs[i];

                if (can_retreat(t1, t2)) {
                    printf(" %llu", (long long unsigned)t2);
                }
            }

            putchar('\n');
        }
    }

    putchar('\n');

    orders_n = 0;
}

int main()
{
    clear_all_units();

    while (yyparse());

    return 0;
}
