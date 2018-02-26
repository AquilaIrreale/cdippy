#include <assert.h>

#include "map.h"

const char *terr_name(enum territory t) {
    static const char *names[75] = {
        /* TODO: names */
    };

    return names[t];
}

bool is_single_coast(enum territory t)
{
    return t != BUL &&
           t != SPA &&
           t != STP;
}

bool is_land(enum territory t)
{
    return is_inner_land(t) || is_coast(t);
}

bool is_sea(enum territory t)
{
    return is_inner_sea(t) || is_coast(t);
}

bool is_coast(enum territory t)
{
    static enum territory coast[] = {
        ALB, ANK, APU, ARM, BEL, BER,
        BRE, BUL, CLY, CON, DEN, EDI,
        FIN, GAS, GRE, HOL, KIE, LON,
        LVN, LVP, MAR, NAF, NAP, NWY,
        PIC, PIE, POR, PRU, ROM, RUM,
        SEV, SMY, SPA, STP, SWE, SYR,
        TRI, TUN, TUS, VEN, WAL, YOR
    };

    size_t i;
    for (i = 0; i < sizeof coast / sizeof coast[0]; i++) {
        if (coast[i] == t) {
            return true;
        }
    }

    return false;
}

bool is_inner_land(enum territory t)
{
    static enum territory inner_land[] = {
        MOS, UKR, WAR, SER, BUD, GAL,
        BOH, VIE, TYR, SIL, MUN, RUH,
        BUR, PAR
    };

    size_t i;
    for (i = 0; i < sizeof inner_land / sizeof inner_land[0]; i++) {
        if (inner_land[i] == t) {
            return true;
        }
    }

    return false;
}

bool is_inner_sea(enum territory t)
{
    static enum territory inner_sea[] = {
        BLA, AEG, EAS, ION, ADR, TYS,
        LYO, WES, MAO, NAO, IRI, ENG,
        NWG, NTH, HEL, SKA, BAL, BOT,
        BAR
    };

    size_t i;
    for (i = 0; i < sizeof inner_sea / sizeof inner_sea[0]; i++) {
        if (inner_sea[i] == t) {
            return true;
        }
    }

    return false;
}

bool is_land_adjacent(enum territory t1,
                      enum territory t2)
{
    struct terr_info *terr = &territories[t1];

    size_t i;
    for (i = 0; i < terr->land_neighs_n; i++) {
        if (terr->land_neighs[i] == t2) {
            return true;
        }
    }

    return false;
}

bool is_sea_adjacent(enum territory t1,
                     enum territory t2)
{
    struct terr_info *terr = &territories[t1];

    size_t i;
    for (i = 0; i < terr->sea_neighs_n; i++) {
        if (terr->sea_neighs[i] == t2) {
            return true;
        }
    }

    return false;
}

bool is_coast_adjacent(enum territory t1,
                       enum territory t2,
                       enum coast coast)
{
    assert(is_single_coast(t1) &&
           !is_single_coast(t2) &&
           coast != NONE);

    static enum territory adj[2][3][6] = {
        {
            {RUM, BLA, CON, -1},
            {POR, MAO, GAS, -1},
            {BAR, NWG, -1}
        }, {
            {CON, AEG, GRE, -1},
            {POR, MAO, WES, LYO, MAR, -1},
            {FIN, BOT, LVN, -1}
        }
    };

    size_t i = coast == NORTH ? 0 : 1;
    size_t j = t2 == BUL ? 0
             : t2 == SPA ? 1
             : t2 == STP ? 2
             : 3;

    assert(j < 3);

    size_t k;
    for (k = 0; adj[i][j][k] >= 0; k++) {
        if (adj[i][j][k] == t1) {
            return true;
        }
    }

    return false;
}

bool can_reach(enum territory t1,
               enum territory t2,
               enum coast coast)
{
    struct terr_info *terr = &territories[t1];

    if (!terr->occupied) {
        return false;
    }

    if (terr->unit == ARMY) {
        return is_land_adjacent(t1, t2);
    }

    bool sea_adjacent = is_sea_adjacent(t1, t2);

    if (!sea_adjacent) {
        return false;
    }

    if (coast == NONE) {
        assert(is_single_coast(t2));

        if (is_single_coast(t1)) {
            return sea_adjacent;
        }

        return is_coast_adjacent(t2, t1, terr->coast);
    }

    assert(!is_single_coast(t2) && is_single_coast(t1));

    return is_coast_adjacent(t1, t2, coast);
}

bool can_support(enum territory t1,
                 enum territory t2)
{
    struct terr_info *terr = &territories[t1];

    if (!terr->occupied) {
        return false;
    }

    if (terr->unit == ARMY) {
        return is_land_adjacent(t1, t2);
    } else {
        return is_sea_adjacent(t1, t2);
    }
}
