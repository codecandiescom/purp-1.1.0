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

#ifndef TOOLS_H
#define TOOLS_H

#define FATAL      1
#define NOT_FATAL  0

#include "linklist.h"
#include "config.h"

void *xmalloc( size_t size );
void fatal( char *ptr );
int TermWrite( char *message, ...);
void DumpListTerm( listelement *lListpointer, short );
void LockTTY( short );
char * StrToLower( char * string );
short IsRPM( char * name );
void ErrorMessage( char * message, bool quit_purp );


#endif // TOOLS_H
