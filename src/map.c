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

const char *cd_nation_names[] = {
    "AUSTRIA",  "ENGLAND", "FRANCE",
    "GERMANY", "ITALY", "RUSSIA", "TURKEY"
};

const char *cd_terr_names[] = {
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
    "StP", "Swe", "Syr", "Tri", "Tun", "Tus",
    "Tyr", "TYS", "Ukr", "Ven", "Vie", "Wal",
    "War", "WES", "Yor"
};

int cd_register_unit(enum cd_terr terr,
                     enum cd_coast coast,
                     enum cd_unit unit,
                     enum cd_nation nation)
{
    if (terr < 0 || terr > TERR_N) {
        return CD_INVALID_TERR;
    }

    bool single = cd_is_single_coast(terr);

    if (single && coast != NO_COAST) {
        return CD_SINGLE_COAST;
    }

    if (!single && unit == FLEET && coast == NO_COAST) {
        return CD_COAST_NEEDED;
    }

    if (unit != FLEET && coast != NO_COAST) {
        return CD_COAST_FOR_ARMY;
    }

    if (unit == ARMY && cd_is_inner_sea(terr)) {
        return CD_ARMY_IN_SEA;
    }

    if (unit == FLEET && cd_is_inner_land(terr)) {
        return CD_FLEET_ON_LAND;
    }

    territories[terr].occupied = true;
    territories[terr].unit = unit;
    territories[terr].nation = nation;
    territories[terr].coast = coast;

    return 0;
}

void cd_register_unit_internal(enum cd_terr terr,
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
        fprintf(stderr, "You are not supposed to specify a coast for %s. Ignored\n", cd_terr_name(terr));
        break;

    case CD_COAST_NEEDED:
        fprintf(stderr, "You are supposed to specify a coast for %s. Ignored\n", cd_terr_name(terr));
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

void cd_clear_unit_internal(enum cd_terr terr)
{
    if (cd_clear_unit(terr)) {
        fputs("Territory out of range. Ignored", stderr);
    }
}

void cd_clear_all_units()
{
    size_t i;
    for (i = 0; i < TERR_N; i++) {
        territories[i].occupied = false;
    }
}

const char *cd_terr_name(enum cd_terr t) {
    if (t < 0 ||
        t >= (sizeof cd_terr_names / sizeof *cd_terr_names)) {
        return "<not a territory>";
    }

    return cd_terr_names[t];
}

bool cd_is_single_coast(enum cd_terr t)
{
    return t != BUL &&
           t != SPA &&
           t != STP;
}

bool cd_is_land(enum cd_terr t)
{
    return cd_is_inner_land(t) || cd_is_coast(t);
}

bool cd_is_sea(enum cd_terr t)
{
    return cd_is_inner_sea(t) || cd_is_coast(t);
}

bool cd_is_coast(enum cd_terr t)
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

bool cd_is_inner_land(enum cd_terr t)
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

bool cd_is_inner_sea(enum cd_terr t)
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

bool cd_is_land_adjacent(enum cd_terr t1,
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

bool cd_is_sea_adjacent(enum cd_terr t1,
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

bool cd_is_coast_adjacent(enum cd_terr t1,
                       enum cd_terr t2,
                       enum cd_coast coast)
{
    assert(cd_is_single_coast(t1) &&
           !cd_is_single_coast(t2) &&
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

bool cd_can_reach(enum cd_terr t1,
               enum cd_terr t2,
               enum cd_unit unit,
               enum cd_coast coast)
{
    if (unit == ARMY) {
        return cd_is_land_adjacent(t1, t2);
    }

    bool sea_adjacent = cd_is_sea_adjacent(t1, t2);

    if (!sea_adjacent) {
        return false;
    }

    if (coast == NO_COAST) {
        if (!cd_is_single_coast(t2)) {
            return false;
        }

        if (cd_is_single_coast(t1)) {
            return sea_adjacent;
        }

        coast = territories[t1].coast;

        return cd_is_coast_adjacent(t2, t1, coast);
    }
                           /*      Unnecessary       */
    if (cd_is_single_coast(t2)/* || !cd_is_single_coast(t1)*/) {
        return false;
    }

    return cd_is_coast_adjacent(t1, t2, coast);
}

bool cd_can_support(enum cd_terr t1,
                 enum cd_terr t2,
                 enum cd_unit unit,
                 enum cd_coast coast)
{
    if (unit == ARMY) {
        return cd_can_reach(t1, t2, unit, coast);
    }

    if (cd_is_single_coast(t1)) {
        return cd_is_sea_adjacent(t1, t2);
    } else if (!cd_is_single_coast(t1) && cd_is_single_coast(t2)) {
        return cd_can_reach(t2, t1, unit, coast);
    } else {
        return false;
    }
}
