#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "map.h"

void register_unit(enum territory terr,
                   enum coast coast,
                   enum unit unit,
                   enum nation nation)
{
    if (terr < 0 || terr > TERR_N) {
        fputs("Territory out of range. Ignored", stderr);
        return;
    }

    bool single = is_single_coast(terr);

    if (single && coast != NONE) {
        fprintf(stderr, "You are not supposed to specify a coast for %s. Ignored",
        terr_name(terr));
        return;
    }

    if (!single && unit == FLEET && coast != NONE) {
        fprintf(stderr, "You are supposed to specify a coast for %s. Ignored",
        terr_name(terr));
        return;
    }

    if (unit != FLEET && coast != NONE) {
        fputs("You are not supposed to specify a coast if unit is an army. Ignored", stderr);
        return;
    }

    territories[terr].occupied = true;
    territories[terr].unit = unit;
    territories[terr].nation = nation;
    territories[terr].coast = coast;
}

void clear_unit(enum territory terr)
{
    if (terr < 0 || terr > TERR_N) {
        fputs("Territory out of range. Ignored", stderr);
        return;
    }

    territories[terr].occupied = false;
}

const char *terr_name(enum territory t) {
    static const char *names[TERR_N] = {
        /* TODO: names */
    };

    return "<Placeholder>";
}

enum unit get_unit(const char *s)
{
    if (strlen(s) != 1) {
        return -1;
    }

    char c = tolower(s[0]);

    return c == 'f' ? FLEET
         : c == 'a' ? ARMY
         : -1;
}

enum coast get_coast(const char *s)
{
    char buf[2];
    size_t l = strlen(s);
    if (l == 2) {
        buf[0] = tolower(s[0]);
        buf[1] = tolower(s[1]);
    } else if (l == 4) {
        if (s[0] != '(' || s[3] != ')') {
            return -1;
        }

        buf[0] = tolower(s[1]);
        buf[1] = tolower(s[2]);
    } else {
        return -1;
    }

    if (buf[0] == 'n' && buf[1] == 'c') {
        return NORTH;
    } else if(buf[0] == 's' && buf[1] == 'c') {
        return SOUTH;
    } else {
        return -1;
    }
}

enum territory get_territory(const char *s)
{
    static const char *names[TERR_N] = {
        "adr", "aeg", "alb", "ank", "apu", "arm",
        "bal", "bar", "bel", "ber", "bla", "boh",
        "bot", "bre", "bud", "bul", "bur", "cly",
        "con", "den", "eas", "edi", "eng", "fin",
        "gal", "gas", "gre", "hel", "hol", "ion",
        "iri", "kie", "lon", "lvn", "lvp", "lyo",
        "mao", "mar", "mos", "mun", "naf", "nao",
        "nap", "nth", "nwg", "nwy", "par", "pic",
        "pie", "por", "pru", "rom", "ruh", "rum",
        "ser", "sev", "sil", "ska", "smy", "spa",
        "stp", "swe", "syr", "tri", "tun", "tus",
        "tyr", "tys", "ukr", "ven", "vie", "wal",
        "war", "wes", "yor"
    };

    if (strlen(s) != 3) {
        return -1;
    }

    char lower[4];

    char *c = lower;
    while ((*c++ = tolower(*s++)));

    enum territory i;
    for (i = 0; i < TERR_N; i++) {
        if (strcmp(names[i], lower) == 0) {
            return i;
        }
    }

    return -1;
}

enum nation get_nation(const char *s)
{
    static const char *names[TERR_N] = {
        "austria", "france", "germany",
        "italy", "russia", "turkey", "uk"
    };

    if (strlen(s) > 7) {
        return -1;
    }

    char lower[8];

    char *c = lower;
    while ((*c++ = tolower(*s++)));

    enum nation i;
    for (i = 0; i < 7; i++) {
        if (strcmp(names[i], lower) == 0) {
            return i;
        }
    }

    return -1;
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
