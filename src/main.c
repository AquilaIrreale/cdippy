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

    orders_n = 0;
}

int main(int argc, char *argv[])
{
    while (yyparse()) {
        puts("asdasdasd");
    }

    return 0;
}
