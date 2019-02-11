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

#include <curses.h>
#include <string.h>

#include "main.h"
#include "linklist.h"
#include "dir.h"
#include "tools.h"

#include "textwindow.h"
#include "pathwindow.h"

extern listelement *listpointer[];
extern int new_option[];
extern char *availdir;
extern int panel;  // the currently selected panel


pathwindow::pathwindow( int _x, int _y, int _width )
{
   x = _x;  y = _y;
   height = 1;
   width = _width;
   
   win = newwin( height, width, y, x );
   if( win == NULL )
    {   
       endwin();
       ErrorMessage( "Unable to create path-window!", TRUE );
    }

   /* Set background-color */
   wbkgd( win, COLOR_PAIR(1) );
   
   /* Set colors */
   wattrset( win, COLOR_PAIR(1) );
   
   /* Erase window */
   werase( win );
}


void pathwindow::PrintPath( char * path )
{
   char tmpstr[255];
   
   werase( win );   
   mvwaddch( win, 0, 0, ACS_VLINE );
   mvwaddch( win, 0, width-1, ACS_VLINE );
   
   mvwaddstr( win, 0, 2, path );

   wrefresh( win );   
}


void pathwindow::Refresh( void )
{
   /* Refresh it! */
   mvwaddch( win, 0, 0, ACS_VLINE );
   mvwaddch( win, 0, width-1, ACS_VLINE );
   wrefresh( win );
}         


short pathwindow::ShowText( char * _text )
{
   if( _text != NULL )
     strcpy( text, _text );

   mvwaddstr( win, 0, 2, "                                                                                                                                                                                     " );
   mvwaddstr( win, 0, 2, text );
   Refresh();
}       



void pathwindow::Display( listelement *listItem )
{
   char tmpstr[255];

   if( listItem->elementtype == GROUP )
     sprintf( tmpstr, "%s", listItem->group );
   else
     sprintf( tmpstr, "/%s/%s-%s-%s", listItem->group,
	     listItem->name,
	     listItem->version,
	     listItem->release );

   ShowText( tmpstr );
}
	
