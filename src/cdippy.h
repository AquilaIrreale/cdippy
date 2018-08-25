#ifndef _CDIPPY_H_
#define _CDIPPY_H_

#define MAX_ORDERS 34

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
    NO_NATION =  0,
    AUSTRIA   =  1,
    ENGLAND   =  2,
    FRANCE    =  4,
    GERMANY   =  8,
    ITALY     = 16,
    RUSSIA    = 32,
    TURKEY    = 64
};

enum territory {
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

enum resolution {
    FAILS,
    SUCCEEDS
};

extern enum resolution resolutions[34];

struct retreat {
    enum territory who;
    enum territory where[16];
    size_t where_n;
};

extern struct retreat retreats[MAX_ORDERS];
extern size_t retreats_n;

#endif /* _CDIPPY_H_ */
