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

%{
#include <config.h>

#include <ctype.h>

#include "map.h"
#include "parser.h"

enum cd_unit get_unit(const char *s);
enum cd_coast get_coast(const char *s);
enum cd_terr get_territory(const char *s);
enum cd_nation get_nation(const char *s);
%}

%%

[ \r\t\v\f\b]+ {; /* Ignore */}

[Cc][Ll][Ee][Aa][Rr]" "[Aa][Ll][Ll] {return CLEAR_ALL;}

[Cc][Ll][Ee][Aa][Rr] {return CLEAR;}

\([NSns][Cc]\) {
    yylval.i = get_coast(yytext);
    return COAST;
}

[A-Za-z]+ {
    int ret;

    switch (yyleng) {
    case 1:
        ;
        char l = tolower(yytext[0]);
        switch (l) {
        case 's':
            return S;
        case 'c':
            return C;
        }

        ret = UNIT;
        yylval.i = get_unit(yytext);
        break;

    case 2:
        ret = COAST;
        yylval.i = get_coast(yytext);
        break;

    case 3:
        ret = TERRITORY;
        yylval.i = get_territory(yytext);
        break;

    default:
        ret = NATION;
        yylval.i = get_nation(yytext);
    }

    if (yylval.i < 0) {
        yylval.s = yytext;
        return UNRECOGNIZED;
    }

    return ret;
}

.|\n {return yytext[0];}

%%

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
    static const char *names[] = {
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
    static const char *names[] = {
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
