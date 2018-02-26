#ifndef _MAP_H_
#define _MAP_H_

#include <stddef.h>
#include <stdbool.h>

enum unit {
    ARMY,
    FLEET
};

enum coast {
    NONE,
    NORTH,
    SOUTH
};

enum nation {
    AUSTRIA,
    FRANCE,
    GERMANY,
    ITALY,
    RUSSIA,
    TURKEY,
    UK
};

enum territory {
    ADR, AEG, ALB, ANK, APU, ARM,
    BAL, BAR, BEL, BER, BLA, BOH,
    BOT, BRE, BUD, BUL, BUR, CLY,
    CON, DEN, EAS, EDI, ENG, FIN,
    GAL, GAS, GRE, HEL, HOL, ION,
    IRI, KIE, LON, LVN, LVP, LYO,
    MAO, MAR, MOS, MUN, NAF, NAO,
    NAP, NTH, NWG, NWY, PAR, PIC,
    PIE, POR, PRU, ROM, RUH, RUM,
    SER, SEV, SIL, SKA, SMY, SPA,
    STP, SWE, SYR, TRI, TUN, TUS,
    TYR, TYS, UKR, VEN, VIE, WAL,
    WAR, WES, YOR
};

const char *terr_name(enum territory t);

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

extern struct terr_info territories[75];

bool is_single_coast(enum territory t);

bool is_land(enum territory t);
bool is_sea(enum territory t);
bool is_coast(enum territory t);
bool is_inner_land(enum territory t);
bool is_inner_sea(enum territory t);

bool can_reach(enum territory t1,
               enum territory t2,
               enum coast coast);
bool can_support(enum territory t1,
                enum territory t2);

#endif /* _MAP_H_ */
