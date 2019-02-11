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

#include "linklist.h"

#include "progresswindow.h"
#include "tools.h"

extern listelement *listpointer[];
extern int new_option[];


progresswindow::progresswindow( void )
{
   strcpy( title, "" );
   maxval = 0;
   curval = 0;

   InitWindow();
}


progresswindow::progresswindow( int max, int start )
{
   strcpy( title, "" );
   maxval = max;
   curval = start;

   InitWindow();
}


progresswindow::progresswindow( char * ltitle, int max, int start ) 
{
   strcpy( title, ltitle );
   maxval = max;
   curval = start;

   InitWindow();
}


void progresswindow::InitWindow( void )
{
   char tmpStr[66];
   int count, maxx;
   
   width = 60;
   height = 5;
   
   x = (COLS - width) / 2;
   y = 6;
   
   win = newwin( height, width, y, x );
   if( win == NULL )
    {   
       fprintf( stderr, "Error: Unable to create progressbar-window!\n" );
       endwin();
       exit(0);
    }


   wbkgd( win, COLOR_PAIR(2) );
   wattrset( win, COLOR_PAIR(2) );
   werase( win );   
   
   
   wborder( win, 0, 0, 0, 0, 0, 0, 0, 0);

   if( strcmp( title, "" ) != 0 )
     {
	sprintf( tmpStr, " %s ", title );
	mvwaddch( win, 0, 2, ACS_RTEE );
	mvwaddstr( win, 0, 3, tmpStr );
	mvwaddch( win, 0, strlen( tmpStr ) + 3, ACS_LTEE );
     }

   wattrset( win, A_REVERSE); 
   
   maxx = (curval*(width-4))/maxval;
   for( count = 2; count < maxx; count++ )
     mvwaddch( win, 2, count, ' ' );
     
   wrefresh( win );
}


void progresswindow::Reset( char * ltitle, int max, int start ) 
{
   char tmpStr[66];
   int count, maxx;

   strcpy( title, ltitle );
   maxval = max;
   curval = start;
   
   width = 60;
   height = 5;
   
   wbkgd( win, COLOR_PAIR(2) );
   wattrset( win, COLOR_PAIR(2) );
   werase( win );   
      
   wborder( win, 0, 0, 0, 0, 0, 0, 0, 0);

   if( strcmp( title, "" ) != 0 )
     {
	sprintf( tmpStr, " %s ", title );
	mvwaddch( win, 0, 2, ACS_RTEE );
	mvwaddstr( win, 0, 3, tmpStr );
	mvwaddch( win, 0, strlen( tmpStr ) + 3, ACS_LTEE );
     }

   wattrset( win, A_REVERSE); 

   maxx = (curval*(width-4))/maxval;
   for( count = 2; count < maxx; count++ )
     mvwaddch( win, 2, count, ' ' );
     
   wrefresh( win );
}


/* Change the current value to 'curval += incval' and redraw */
int progresswindow::Update( int incval )
{
   int count, oldcur;
   int startx, stopx;

   /* Change curval */
   oldcur = curval; 
   curval += incval;

   /* Check if we decrease or increase the progressbar */
   if( incval < 0 )
     {
	startx = ( curval * ( width - 4 ) ) / maxval; 
	stopx  = ( oldcur * ( width - 4 ) ) / maxval;
	wattrset( win, A_NORMAL);  /* Erase blocks */
     }
   else
     {
	startx = ( oldcur * ( width - 4 ) ) / maxval; 
	stopx  = ( curval * ( width - 4 ) ) / maxval;
	wattrset( win, A_REVERSE);  /* Draw blocks */
     }

   for( ; startx < stopx ; startx++ )
     mvwaddch( win, 2, startx + 2, ' ' );

   wrefresh( win );

   return( curval );
}


/* Set the window-title */
void progresswindow::SetTitle( char * lTitle )
{
   char tmpStr[66];

   wbkgd( win, COLOR_PAIR(2) );
   wattrset( win, COLOR_PAIR(2) );
   
   wborder( win, 0, 0, 0, 0, 0, 0, 0, 0);

   if( strcmp( title, "" ) != 0 )
     {
	sprintf( tmpStr, " %s ", title );
	mvwaddch( win, 0, 2, ACS_RTEE );
	mvwaddstr( win, 0, 3, tmpStr );
	mvwaddch( win, 0, strlen( tmpStr ) + 3, ACS_LTEE );
     }
   
}


/* Change the current value to 'curval += incval' and redraw */
int progresswindow::SetValue( int val )
{
   curval = val;
   
   return( curval );
}

