%{
#include <stdio.h>

#include "map.h"
#include "adjudicator.h"

void execute();
void yyerror(const char *s);
int yywrap();
int yylex();
%}

%union {
    int i;
    const char *s;
}

%token <i> TERRITORY
%token <i> NATION
%token <i> COAST
%token <i> UNIT

%token <s> UNRECOGNIZED

%token CLEAR

%start list

%%

list: state_list '\n' orders_list '\n' {execute();}

state_list: /* Nothing */
          | state_list state_directive '\n'

orders_list: /* Nothing */
           | orders_list order '\n'

state_directive: TERRITORY UNIT NATION {register_unit($1, NONE, $2, $3);}
               | TERRITORY COAST UNIT NATION {register_unit($1, $2, $3, $4);}
               | CLEAR TERRITORY {clear_unit($2);}

order: move_order
     | support_order
     | convoy_order

move_order: TERRITORY '-' TERRITORY       {register_order(MOVE, $1, 0, $3, NONE);}
          | TERRITORY '-' TERRITORY COAST {register_order(MOVE, $1, 0, $3, $4);}

support_order: TERRITORY 's' TERRITORY '-' TERRITORY {register_order(SUPPORT, $1, $3, $5, NONE);}

convoy_order: TERRITORY 'c' TERRITORY '-' TERRITORY {register_order(CONVOY, $1, $3, $5, NONE);}

%%

void yyerror(const char *s)
{
    fputs(s, stderr);
    fputc('\n', stderr);
}

int yywrap()
{
    return 1;
}
