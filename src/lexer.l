%{
#include <ctype.h>

#include "map.h"
#include "parser.h"
%}

%%

[ \r\t\v\f\b]+ {; /* Ignore */}

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
