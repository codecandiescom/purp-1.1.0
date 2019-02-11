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
#include "rpmprogress.h"
#include "tools.h"

extern listelement *listpointer[];
extern int new_option[];



rpmprogress::rpmprogress( char * ltitle, unsigned long total, 
			 int termHeight, int termWidth ) 
{
   char tmpStr[128];

   strcpy( title, ltitle );
   size = 0;
   current_size = 0;

   total_current_size= 0;
   total_blockcount = 0;
   total_size = total;
      
   width = 60;
   height = 9;
   
   x = (termWidth-width)/2;
   y = (termHeight-height)/2;
   
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
     
   wrefresh( win );
}

/* Change progressbar to show info about a new package */
void rpmprogress::Next( char * pname, unsigned long _size ) 
{
   char tmpStr[66];
   int count, maxx;

   size = _size;
   total_current_size += current_size;
   current_size = 0;
   
   wbkgd( win, COLOR_PAIR(2) );
   wattrset( win, COLOR_PAIR(2) );
   werase( win );   
      
   wborder( win, 0, 0, 0, 0, 0, 0, 0, 0);

   /* Print window-title */
   if( strcmp( title, "" ) != 0 )
     {
	sprintf( tmpStr, " %s ", title );
	mvwaddch( win, 0, 2, ACS_RTEE );
	mvwaddstr( win, 0, 3, tmpStr );
	mvwaddch( win, 0, strlen( tmpStr ) + 3, ACS_LTEE );
     }

   /* Print packagename */
   sprintf( tmpStr, "%s (%d KB):", pname, (_size / 1024));
   mvwaddstr( win, 2, 2, tmpStr );

   /* Print total-label */
   sprintf( tmpStr, "Total (%d KB):", (total_size / 1024));
   mvwaddstr( win, 5, 2, tmpStr );
   
   if( total_size )
     {
	int startx, stopx;
	wattrset( win, A_REVERSE); 
	stopx  = ( (total_current_size + current_size) * ( width - 4 ) ) / total_size;
	if( stopx > width-4 ) stopx = width-4;
	
	for( startx = 0; startx < stopx ; startx++ )
	  mvwaddch( win, 6, startx + 2, ' ' );
     }

   wrefresh( win );
}


/* Update and redraw the progressbars */
void  rpmprogress::Update( unsigned long count )
{
   unsigned long old_size;
   int startx, stopx;
   
   /* Change curval */
   old_size = current_size; 
   current_size = count;

   wattrset( win, A_REVERSE);  /* Draw blocks */
 
   startx = ( old_size * ( width - 4 ) ) / size; 
   stopx  = ( current_size * ( width - 4 ) ) / size;
   if( stopx > width-4 ) stopx = width-4;

   for( ; startx < stopx ; startx++ )
     mvwaddch( win, 3, startx + 2, ' ' );

   /* Update total-progress */
   if( (((float) total_current_size + current_size) / total_size * (width-4)) 
                                              >= total_blockcount )
     {
	total_blockcount++;

        startx = ( (total_current_size + old_size) * ( width - 4 ) ) / total_size; 
	stopx  = ( (total_current_size + current_size) * ( width - 4 ) ) / total_size;
	if( stopx > width-4 ) stopx = width-4;
	
	for( ; startx < stopx ; startx++ )
	  mvwaddch( win, 6, startx + 2, ' ' );
     }
   
   wrefresh( win );
}


/* Set the window-title */
void rpmprogress::SetTitle( char * lTitle )
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


/* Add size of a package to the total-size */
void rpmprogress::IncTotal( unsigned long _size )
{
   total_current_size += _size;
}
