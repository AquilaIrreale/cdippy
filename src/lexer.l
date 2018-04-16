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
