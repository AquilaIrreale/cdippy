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

#include <stdio.h>
#include <stdlib.h>

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
%token CLEAR_ALL
%token S
%token C

%start list

%%

list: /* Nothing */
    | list state_list '\n' orders_list '\n' {execute();}

state_list: /* Nothing */
          | state_list state_directive '\n'

orders_list: /* Nothing */
           | orders_list order '\n'

state_directive: TERRITORY UNIT NATION {register_unit($1, NONE, $2, $3);}
               | TERRITORY COAST UNIT NATION {register_unit($1, $2, $3, $4);}
               | CLEAR TERRITORY {clear_unit($2);}
               | CLEAR_ALL {clear_all_units();}

order: move_order
     | support_order
     | convoy_order

move_order: TERRITORY '-' TERRITORY         {register_order(MOVE, $1, 0, $3, NONE, false);}
          | TERRITORY '-' TERRITORY COAST   {register_order(MOVE, $1, 0, $3,   $4, false);}
          | TERRITORY '-' TERRITORY C       {register_order(MOVE, $1, 0, $3, NONE, true);}
          | TERRITORY '-' TERRITORY COAST C {register_order(MOVE, $1, 0, $3,   $4, true);}

support_order: TERRITORY S TERRITORY '-' TERRITORY {register_order(SUPPORT, $1, $3, $5, NONE, false);}
             | TERRITORY S TERRITORY               {register_order(SUPPORT, $1, $3, $3, NONE, false);}

convoy_order: TERRITORY C TERRITORY '-' TERRITORY {register_order(CONVOY, $1, $3, $5, NONE, false);}

%%

void yyerror(const char *s)
{
    fputs(s, stderr);
    fputc('\n', stderr);
    exit(1);
}

int yywrap()
{
    return 1;
}
