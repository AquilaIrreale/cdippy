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

#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "map.h"

int cd_register_unit(enum cd_terr terr,
                     enum cd_coast coast,
                     enum cd_unit unit,
                     enum cd_nation nation)
{
    if (terr < 0 || terr > TERR_N) {
        return CD_INVALID_TERR;
    }

    bool single = is_single_coast(terr);

    if (single && coast != NO_COAST) {
        return CD_SINGLE_COAST;
    }

    if (!single && unit == FLEET && coast == NO_COAST) {
        return CD_COAST_NEEDED;
    }

    if (unit != FLEET && coast != NO_COAST) {
        return CD_COAST_FOR_ARMY;
    }

    if (unit == ARMY && is_inner_sea(terr)) {
        return CD_ARMY_IN_SEA;
    }

    if (unit == FLEET && is_inner_land(terr)) {
        return CD_FLEET_ON_LAND;
    }

    territories[terr].occupied = true;
    territories[terr].unit = unit;
    territories[terr].nation = nation;
    territories[terr].coast = coast;

    return 0;
}

void register_unit(enum cd_terr terr,
                   enum cd_coast coast,
                   enum cd_unit unit,
                   enum cd_nation nation)
{
    int ret = cd_register_unit(terr, coast, unit, nation);

    switch (ret) {
    case CD_INVALID_TERR:
        fputs("Territory out of range. Ignored", stderr);
        break;

    case CD_SINGLE_COAST:
        fprintf(stderr, "You are not supposed to specify a coast for %s. Ignored\n", terr_name(terr));
        break;

    case CD_COAST_NEEDED:
        fprintf(stderr, "You are supposed to specify a coast for %s. Ignored\n", terr_name(terr));
        break;

    case CD_COAST_FOR_ARMY:
        fputs("You are not supposed to specify a coast if unit is an army. Ignored", stderr);
        break;

    case CD_ARMY_IN_SEA:
        fputs("Armies can only be placed on land. Ignored", stderr);
        break;

    case CD_FLEET_ON_LAND:
        fputs("Fleets cannot be placed on dry land. Ignored", stderr);
        break;

    default:
        break;
    }
}

int cd_clear_unit(enum cd_terr terr)
{
    if (terr < 0 || terr > TERR_N) {
        return CD_INVALID_TERR;
    }

    territories[terr].occupied = false;

    return 0;
}

void clear_unit(enum cd_terr terr)
{
    if (cd_clear_unit(terr)) {
        fputs("Territory out of range. Ignored", stderr);
    }
}

void clear_all_units()
{
    size_t i;
    for (i = 0; i < TERR_N; i++) {
        territories[i].occupied = false;
    }
}

const char *terr_name(enum cd_terr t) {
    static const char *names[TERR_N] = {
        "ADR", "AEG", "Alb", "Ank", "Apu", "Arm",
        "BAL", "BAR", "Bel", "Ber", "BLA", "Boh",
        "BOT", "Bre", "Bud", "Bul", "Bur", "Cly",
        "Con", "Den", "EAS", "Edi", "ENG", "Fin",
        "Gal", "Gas", "Gre", "HEL", "Hol", "ION",
        "IRI", "Kie", "Lon", "Lvn", "Lvp", "LYO",
        "MAO", "Mar", "Mos", "Mun", "NAf", "NAO",
        "Nap", "NTH", "NWG", "Nwy", "Par", "Pic",
        "Pie", "Por", "Pru", "Rom", "Ruh", "Rum",
        "Ser", "Sev", "Sil", "SKA", "Smy", "Spa",
        "StP", "Swe", "Syr", "TRI", "Tun", "Tus",
        "Tyr", "TYS", "Ukr", "Ven", "Vie", "Wal",
        "War", "WES", "Yor"
    };

    if (t >= (sizeof names / sizeof *names)) {
        return "<not a territory>";
    }

    return names[t];
}

enum cd_unit get_unit(const char *s)
{
    if (strlen(s) != 1) {
        return -1;
    }

    char c = tolower(s[0]);

    return c == 'f' ? FLEET
         : c == 'a' ? ARMY
         : -1;
}

enum cd_coast get_coast(const char *s)
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

enum cd_terr get_territory(const char *s)
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

    enum cd_terr i;
    for (i = 0; i < TERR_N; i++) {
        if (strcmp(names[i], lower) == 0) {
            return i;
        }
    }

    return -1;
}

enum cd_nation get_nation(const char *s)
{
    static const char *names[TERR_N] = {
        "austria",  "england", "france",
        "germany", "italy", "russia", "turkey"
    };

    if (strlen(s) > 7) {
        return -1;
    }

    char lower[8];

    char *c = lower;
    while ((*c++ = tolower(*s++)));

    enum cd_nation i;
    for (i = 0; i < 7; i++) {
        if (strcmp(names[i], lower) == 0) {
            return 1 << i;
        }
    }

    return -1;
}

bool is_single_coast(enum cd_terr t)
{
    return t != BUL &&
           t != SPA &&
           t != STP;
}

bool is_land(enum cd_terr t)
{
    return is_inner_land(t) || is_coast(t);
}

bool is_sea(enum cd_terr t)
{
    return is_inner_sea(t) || is_coast(t);
}

bool is_coast(enum cd_terr t)
{
    static enum cd_terr coast[] = {
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

bool is_inner_land(enum cd_terr t)
{
    static enum cd_terr inner_land[] = {
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

bool is_inner_sea(enum cd_terr t)
{
    static enum cd_terr inner_sea[] = {
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

bool is_land_adjacent(enum cd_terr t1,
                      enum cd_terr t2)
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

bool is_sea_adjacent(enum cd_terr t1,
                     enum cd_terr t2)
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

bool is_coast_adjacent(enum cd_terr t1,
                       enum cd_terr t2,
                       enum cd_coast coast)
{
    assert(is_single_coast(t1) &&
           !is_single_coast(t2) &&
           coast != NO_COAST);

    static enum cd_terr adj[2][3][6] = {
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
    for (k = 0; adj[i][j][k] != -1; k++) {
        if (adj[i][j][k] == t1) {
            return true;
        }
    }

    return false;
}

bool can_reach(enum cd_terr t1,
               enum cd_terr t2,
               enum cd_unit unit,
               enum cd_coast coast)
{
    if (unit == ARMY) {
        return is_land_adjacent(t1, t2);
    }

    bool sea_adjacent = is_sea_adjacent(t1, t2);

    if (!sea_adjacent) {
        return false;
    }

    if (coast == NO_COAST) {
        if (!is_single_coast(t2)) {
            return false;
        }

        if (is_single_coast(t1)) {
            return sea_adjacent;
        }

        coast = territories[t1].coast;

        return is_coast_adjacent(t2, t1, coast);
    }
                           /*      Unnecessary       */
    if (is_single_coast(t2)/* || !is_single_coast(t1)*/) {
        return false;
    }

    return is_coast_adjacent(t1, t2, coast);
}

bool can_support(enum cd_terr t1,
                 enum cd_terr t2,
                 enum cd_unit unit,
                 enum cd_coast coast)
{
    if (unit == ARMY) {
        return can_reach(t1, t2, unit, coast);
    }

    if (is_single_coast(t1)) {
        return is_sea_adjacent(t1, t2);
    } else if (!is_single_coast(t1) && is_single_coast(t2)) {
        return can_reach(t2, t1, unit, coast);
    } else {
        return false;
    }
}
