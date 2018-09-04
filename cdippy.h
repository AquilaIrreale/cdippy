#ifndef _CDIPPY_H_
#define _CDIPPY_H_

#include <stdbool.h>

#define MAX_ORDERS 34
#define NATIONS_N 7

enum cd_unit {
    ARMY,
    FLEET
};

enum cd_coast {
    NO_COAST = 0,
    NORTH    = 1,
    SOUTH    = 2
};

enum cd_nation {
    NO_NATION =  0,
    AUSTRIA   =  1,
    ENGLAND   =  2,
    FRANCE    =  4,
    GERMANY   =  8,
    ITALY     = 16,
    RUSSIA    = 32,
    TURKEY    = 64
};

enum cd_terr {
    NO_TERR = -1, /* Force signed */
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
    WAR, WES, YOR,
    TERR_N
};

extern const char *cd_nation_names[];
extern const char *cd_terr_names[];

enum cd_resolution {
    FAILS,
    SUCCEEDS
};

extern enum cd_resolution cd_resolutions[34];

struct cd_retreat {
    enum cd_terr who;

    struct {
        enum cd_terr terr;
        unsigned coasts;
    } where[16];

    size_t where_n;
};

extern struct cd_retreat cd_retreats[MAX_ORDERS];
extern size_t cd_retreats_n;

enum {
    CD_INVALID_TERR = 1,
    CD_SINGLE_COAST,
    CD_COAST_NEEDED,
    CD_COAST_FOR_ARMY,
    CD_ARMY_IN_SEA,
    CD_FLEET_ON_LAND
};

int cd_register_unit(enum cd_terr terr,
                     enum cd_coast coast,
                     enum cd_unit unit,
                     enum cd_nation nation);

int cd_clear_unit(enum cd_terr terr);

void cd_register_move(enum cd_terr t2, enum cd_terr t3, enum cd_coast coast, bool via_convoy);
void cd_register_suph(enum cd_terr t1, enum cd_terr t2);
void cd_register_supm(enum cd_terr t1, enum cd_terr t2, enum cd_terr t3);
void cd_register_conv(enum cd_terr t1, enum cd_terr t2, enum cd_terr t3);

void cd_run_adjudicator();

#endif /* _CDIPPY_H_ */
