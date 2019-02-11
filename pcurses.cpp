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

#include "panel.h"
#include "cmdwindow.h"
#include "pathwindow.h"
#include "infowindow.h"

#include "pcurses.h"
#include "tools.h"
#include "main.h"
#include "config/confclass.h"


extern panel *packagePanel[];
extern listelement *topListpointer[2];
extern cmdwindow   *cmdwin;
extern pathwindow  *pathwin;
extern infowindow  *infowin;
extern int openWindows;    /* bit-flags for marking that windows are open */  

/* panels */
extern int termWidth;  extern int termHeight;
extern int panelWidth; extern int panelHeight; 

extern short curses_initiated;

extern Conf * purpconf;         /* Config-class */

/*************************************************
 * Initiate ncurses
 *************************************************/
void InitCurses( void )
{
   /* Initiate curses */
   initscr();
   cbreak();
   noecho();
   nonl();
   intrflush( stdscr, FALSE );
   keypad( stdscr, TRUE );

   getmaxyx(stdscr, termHeight, termWidth);

   /* Mark that curses now are initiated, no windows opened */
   openWindows = 0;
   
   /* Check that the terminal is big enough, else quit */
   if( (termHeight < 14) || (termWidth < 60) )
     {
	quitPurp( 667, MINSIZE_MESSAGE );
     }
   
   /* Initiate color if available */
   if( has_colors() )
     {
	/* Initiate color */
	start_color();

	init_pair( 1,  CheckColorValue("textcolor", COLOR_WHITE),
		       CheckColorValue("backcolor", COLOR_BLUE));
	init_pair( 2,  CheckColorValue("dialogtextcolor", COLOR_WHITE),
		       CheckColorValue("dialogbackcolor", COLOR_BLACK));
	init_pair( 3,  CheckColorValue("markcolor", COLOR_GREEN),
		       CheckColorValue("backcolor", COLOR_BLUE));
	init_pair( 4,  COLOR_YELLOW, COLOR_BLUE );
	init_pair( 5,  CheckColorValue("menucolor", COLOR_GREEN), COLOR_BLACK );
	init_pair( 6,  COLOR_RED, COLOR_BLACK );
	init_pair( 7,  COLOR_BLACK, CheckColorValue("backcolor", COLOR_BLUE) );
     }

   /* Set background-color */
   bkgd( COLOR_PAIR( 2 ) );

   /* Clear screen */
   erase();
   
   /* Hide cursor (unfortunatelly not possible for all terminals) */
   curs_set( 0 );
  
   refresh();
   
   curses_initiated = 1;
}


void EndCurses( void )
{
   /* Show cursor (if possible..) */
   curs_set( 1 );

   DeleteWindows();
   
   /* Close curses */
   endwin();     
   
   curses_initiated = 0;
}




void CreateWindows( void )
{
   panelWidth = termWidth / 2;
   panelHeight = termHeight - 9;
   
   /* Create installed-panel */
   packagePanel[INSTALLED] = new panel( INSTALLED, INSTALLED_PANEL_TITLE,
			       0, 0, panelWidth, panelHeight );
   packagePanel[INSTALLED]->SetBorder( 0, 0, 0, 0, 0, 0, ACS_LTEE, 0 );
   packagePanel[INSTALLED]->Refresh();
   openWindows += INSTALLED_PANEL;                
   
   /* Create available-panel */
   packagePanel[AVAILABLE] = new panel( AVAILABLE, AVAILABLE_PANEL_TITLE, 
			       panelWidth, 0, termWidth - panelWidth, panelHeight );
   packagePanel[AVAILABLE]->SetBorder( 0, 0, 0, 0, 0, 0, 0, ACS_RTEE );
   packagePanel[AVAILABLE]->Refresh();
   openWindows += AVAILABLE_PANEL;
   
   /* Create path-window */
   pathwin = new pathwindow( 0, packagePanel[INSTALLED]->Height(), termWidth );
   pathwin->Refresh();
   pathwin->ShowText( "" ); 
   openWindows += PATH_WIN;
   
   /* Create info-window */
   infowin = new infowindow( 0, pathwin->Y() + pathwin->Height(), termWidth,
			    termHeight - pathwin->Y() - pathwin->Height() -1 );
   infowin->SetBorder( 0, 0, 0, 0, ACS_LTEE, ACS_RTEE, 0, 0 );
   infowin->Refresh();
   openWindows += INFO_WIN;
   
   /* Create Command-window */
   cmdwin = new cmdwindow( infowin->Y() + infowin->Height(), termWidth );
   cmdwin->Refresh();
   cmdwin->Draw( ABOUTLINE );
   openWindows += CMD_WIN;                
}


void DeleteWindows( void )
{
   /* Delete panels */
   if( openWindows & INSTALLED_PANEL )
     delete packagePanel[INSTALLED];   openWindows -= INSTALLED_PANEL;

   if( openWindows & AVAILABLE_PANEL )
     delete packagePanel[AVAILABLE];   openWindows -= AVAILABLE_PANEL;
   
   if( openWindows & PATH_WIN )
     delete pathwin;          openWindows -= PATH_WIN;
   
   if( openWindows & INFO_WIN )
     delete infowin;          openWindows -= INFO_WIN;
   
   if( openWindows & CMD_WIN )
     delete cmdwin;           openWindows -= CMD_WIN;
}


/* Check that the color value lies within acceptable limits */
int CheckColorValue( char *item, int value )
{
   static struct
     {
	char *color;
	chtype value;
     } colormap[] = {{"black", COLOR_BLACK}, {"blue", COLOR_BLUE},
	             {"green", COLOR_GREEN}, {"cyan", COLOR_CYAN},
	             {"red", COLOR_RED}, {"magenta", COLOR_MAGENTA},
	             {"yellow", COLOR_YELLOW}, {"white", COLOR_WHITE}};

   char *result = purpconf->Search(item, "");
   
   for (int i = 0; i < 8; i++)
     {
	if (strcmp(result, colormap[i].color) == 0)
	  {
	     value = colormap[i].value;
	     break;
	  }
     }
   return value;
}
