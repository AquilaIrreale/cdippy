#include <stdio.h>

#include "map.h"
#include "adjudicator.h"

enum resolution resolve(size_t o);

int main(int argc, char *argv[])
{
    territories[NAP].occupied = true;
    territories[NAP].unit = ARMY;
    territories[NAP].nation = ITALY;

    territories[ROM].occupied = true;
    territories[ROM].unit = ARMY;
    territories[ROM].nation = AUSTRIA;

    territories[APU].occupied = true;
    territories[APU].unit = ARMY;
    territories[APU].nation = ITALY;

    orders[0].kind = MOVE;
    orders[0].terr = NAP;
    orders[0].targ = ROM;
    orders[0].coast = NONE;

    orders[1].kind = MOVE;
    orders[1].terr = ROM;
    orders[1].targ = NAP;
    orders[1].coast = NONE;

    orders[2].kind = SUPPORT;
    orders[2].terr = APU;
    orders[2].orig = NAP;
    orders[2].targ = ROM;
    orders[2].coast = NONE;

    orders_n = 3;

    if (resolve(0) == SUCCEEDS) {
        puts("Yay!");
    } else {
        puts("Yikes!");
    }
}
