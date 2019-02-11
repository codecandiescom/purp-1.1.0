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
#include <time.h>
#include <string.h>

#include "infowindow.h"
#include "panel.h"

extern int       currentPanel;


/* Constructor for 'panel' */
infowindow::infowindow( int lX, int lY, int lWidth, int lHeight )
{
   /* Copy arguments */
   x = lX; y = lY;
   width = lWidth;
   height = lHeight;

   /* Create curses-window */
   win = newwin( height, width, y, x );
   if( win == NULL )
     {
	fprintf( stderr, "Error: Unable to create info-window!\n" );
	endwin();
	exit(0);
     }
   
   /* Set background-color */
   wbkgd( win, COLOR_PAIR(1) );
   
   /* Set colors */
   wattrset( win, COLOR_PAIR(1) );
   
   /* Erase window */
   werase( win );
}



void infowindow::Refresh( void )
{
   char tmpStr[128];
   /* Draw border */
   wborder( win, ls, rs, ts, bs, tl, tr, bl, br );    

   mvwaddstr( win, 1, 2, "Name:        ");
   mvwaddstr( win, 2, 2, "Version:     ");
   mvwaddstr( win, 3, 2, "Release:     ");
   mvwaddstr( win, 5, 2, "Summary:     ");

   mvwaddstr( win, 1, 40, "Size:        ");
   mvwaddstr( win, 2, 40, "Installdate: ");
   mvwaddstr( win, 3, 40, "Builddate: ");

   /* Refresh it! */
   wrefresh( win );
}


void infowindow::Display( listelement *listItem )
{
   char *empty_string = "                                                                                ";
   char tmpstr[255];

   mvwaddnstr( win, 1, 15, empty_string, 24 );  /* Name */
   mvwaddnstr( win, 1, 15, listItem->name, 24 );

   mvwaddnstr( win, 2, 15, empty_string, 16 );  /* Version */
   mvwaddnstr( win, 3, 15, empty_string, 10 );  /* Release */
   mvwaddnstr( win, 5, 15, empty_string, 60 );  /* Summary */
   mvwaddnstr( win, 1, 48, empty_string, 25 );  /* Size */
   mvwaddnstr( win, 2, 53, empty_string, 25 );  /* Installdate */
   mvwaddnstr( win, 3, 53, empty_string, 25 );  /* Builddate */
	
   if( listItem->elementtype != GROUP )
     {
	/* Print name, version, release */
	mvwaddnstr( win, 2, 15, listItem->version, 16 );
	mvwaddnstr( win, 3, 15, listItem->release, 10 );
	
	/* Print summary */
	if( strlen( listItem->summary ) > 0 )
	  mvwaddnstr( win, 5, 15, listItem->summary, 60 );
	
	/* Print size */
	sprintf(tmpstr, "%d", listItem->size);
	mvwaddnstr( win, 1, 53, tmpstr, 20 );
	
	/* Print either installdate or builddate */
	if( currentPanel == INSTALLED )
	  {
	     mvwaddnstr( win, 2, 53, ctime( &listItem->installdate ), 24 );
	     mvwaddnstr( win, 3, 53, ctime( &listItem->builddate ), 24 );
	  }
	else
	  {
	     if( listItem->installdate != (time_t) 0 ) mvwaddnstr( win, 2, 53, ctime( &listItem->installdate ), 24 );
	     mvwaddnstr( win, 3, 53, ctime( &listItem->builddate ), 24 );
	  }
     }
   Refresh();
}

