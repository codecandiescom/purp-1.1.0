/****************************************************************************
 This file is a part of Purp (Pugo RPM) - a ncurses-based RPM-handler
 Copyright (C) 1998-2001 -- Anders Karlsson <pugo@pugo.org>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.         
 
 For latest information and version of Purp, look at the Purp homepage:
 <http://www.lysator.liu.se/purp/>
 
 Please send comments and bug-reports to: <pugo@pugo.org>
****************************************************************************/

%{
#include <stdio.h>
#include "main.h"
#include "confclass.h"
int yylex();
void yyerror(char *);
extern Conf * purpconf;
%}

%token NUMBER STRING

%%

end:
   | end confline
   | end groupline
   {  }
;


confline: STRING '=' STRING   { purpconf->Add( (char *)$1, (char *)$3 ); }
				
        | STRING '=' NUMBER   { purpconf->Add( (char *)$1, (char *)$3 ); }
;
	
groupline: '[' STRING ']' { }
;

     
%%

extern FILE *yyin;



void yyerror(char *s)
{
   fprintf( stderr, "%s\n", s );
}


