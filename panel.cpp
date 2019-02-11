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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "panel.h"
#include "linklist.h"
#include "tools.h"

extern int         currentPanel;
extern listelement *topListpointer[2];
extern bool dimmed;
extern char *availdir;

/******************************************************************************/


/* Constructor for 'panel' */
panel::panel( short ID, char *_title, int _x, int _y, int _width, int _height )
{
   /* Copy arguments */
   x = _x; y = _y;
   width = _width;
   height = _height;
   
   /* Copy window-title */
   strcpy( title, _title );
   
   selected_count = 0;
   selected_size = 0;
   highest_item_on_list = 0;
   maxItems = 0;
   currentItem = 0;
   panelID = ID;

   /* Create curses-window */
   win = newwin( height, width, y, x );
   if( win == NULL )
     {
	fprintf( stderr, "Error: Unable to create window!\n" );
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


/******************************************************************************/


/* Destructor for panel */
panel::~panel( void )
{
#ifdef DEBUG_TO_TERM
   TermWrite( "panel::~panel (destructor)\n" );
#endif
}



/******************************************************************************/


/* Store handle (pointer to pointer) for the list 
 * that this panel is supposed to show */
void panel::SetList( listelement * _list )
{
   list = _list;

   /* Scan current folder */
   ScanFolder();
}


/******************************************************************************/


int panel::GetCurrentItem( void )
{
   return currentItem;
}


int panel::GetHighestItem( void )
{
   return highest_item_on_list;
}


/******************************************************************************/
/* These are only used when changing group */

void panel::SetCurrentItem( int value )
{
   if( value >= 0 )
     currentItem = value;
}


void panel::SetHighestItem( int value )
{
   highest_item_on_list = value;
}

/******************************************************************************/

void panel::ChangeSelectedCount( int value )
{
   if( value == 0 )
     selected_count = 0;
   else if( selected_count + value >= 0 )
     selected_count += value;
}

void panel::ChangeSelectedSize( unsigned long value )
{ 
   if( value == 0 )
     selected_size = 0;
   else if( selected_size + value >= 0 )
     selected_size += value;
}


int  panel::GetSelectedCount( void )
{
   return selected_count;
}

unsigned long  panel::GetSelectedSize( void )
{
   return selected_size;
}

/******************************************************************************/


/* Refresh (redraw) the panel */
void panel::Refresh( void )
{
   char tmpStr[128];
   
   wattrset( win, COLOR_PAIR(1)  );   

   /* Draw border */
   wborder( win, ls, rs, ts, bs, tl, tr, bl, br );    
 
   /* Draw title */
   if( strcmp( title, "" ) != 0 )
     {
	sprintf( tmpStr, " %s ", title );
	mvwaddch( win, 0, 2, ACS_RTEE );
	mvwaddstr( win, 0, 3, tmpStr );
	mvwaddch( win, 0, strlen( tmpStr ) + 3, ACS_LTEE );
     }         

   /* Draw # selected packages and total selected size */
   sprintf( tmpStr, " (%d / %d KB) ", selected_count, selected_size / 1024 );
   mvwaddstr( win, height - 1, width - strlen(tmpStr) - 2, tmpStr );

   /* Refresh it! */
   wrefresh( win );

}


/******************************************************************************/


/* Run this if we change folder to rescan numbers of 
 * packages, and so on */
void panel::ScanFolder( void )
{
   listelement *localListPtr = list;
   
   if( list )
     if( localListPtr->link != NULL )
       localListPtr = (listelement *)localListPtr->link;
   
   maxItems = 0;
   while( localListPtr != NULL )
     {
	maxItems++;
	localListPtr = (listelement *)localListPtr->link;
     }
}


/******************************************************************************
 * Display packages in the panel
 * Return pointer to the element marked by the cursor
 ******************************************************************************/

listelement * panel::Display( void )
{
   listelement *localListPtr = list;
   listelement *returnPtr = NULL;
   int count = 0;
   char tmpstr[255];
   int avail_color;
   short already_installed = 0;

   if( topListpointer[ panelID ]->straightlink != NULL )
     {
     
	/* Jump to the element after the dummy.. */
	if( localListPtr != NULL ) localListPtr = (listelement *)localListPtr -> link;
	
	/* Move to the first item we want to show... */
	while( count < highest_item_on_list )
	  {
	     localListPtr = (listelement *)localListPtr -> link;
	     count++;
	  }
	
	count = 1;
	
	
	/* Fill the panel with packets */
	while( (localListPtr != NULL) && (count <= height-2) )
	  {     
	     
	     if( localListPtr->elementtype != GROUP && 
		 localListPtr->installdate != (time_t) 0 &&
		 panelID == AVAILABLE) 
	       {
		  avail_color = (dimmed ? COLOR_PAIR(7) | A_BOLD : COLOR_PAIR(1));
		  already_installed = 1;
	       }
	     else 
	       {
		  avail_color = COLOR_PAIR(1);
		  already_installed = 0;
	       }
	     
	     wattrset( win, avail_color );
	     
	     if( localListPtr->is_marked )
	       {
		  wattrset( win, A_BOLD ); 
		  wattrset( win, COLOR_PAIR(3) ); 
	       }
	     
	     /* If current panel & currentItem: draw it as cursor! */
	     if( (currentPanel == panelID) && (currentItem-highest_item_on_list == count-1) )
	       {
		  wattrset( win, A_REVERSE);
		  returnPtr = localListPtr;
	       }

	     
	     /* Calculate the package-names */
	     if( localListPtr->elementtype == GROUP )
	       sprintf( tmpstr, "%s/\n", localListPtr->name );
	     else if( !already_installed )
	       {
		  sprintf( tmpstr, "%s%s-%s-%s\n", 
			  (localListPtr->is_marked == TRUE)?"*":" ",
			  localListPtr->name, 
			  localListPtr->version, 
			  localListPtr->release );
	       }
	     else
	       {
		  sprintf( tmpstr, "(%s-%s-%s)\n", 
			  localListPtr->name, 
			  localListPtr->version, 
			  localListPtr->release );
	       }
	     
	     mvwaddstr( win, count + y, 1, tmpstr );
	     localListPtr = (listelement *)localListPtr -> link;
	     count++;
	  }
		
     }
   Refresh();  /* Redraw borders, title, counter.. */
   return returnPtr;
}


/******************************************************************************
 * Move cursor, up if negative, down if positive.
 * Return pointer to the element marked by the cursor
 ******************************************************************************/
listelement * panel::MoveCursor( int movement )
{
   int pos;
     
   /* First move cursor */
   if( movement > 0 )
     {
	/* Check if we are at the end of the list */
	if( currentItem + movement >= maxItems )
	  currentItem = maxItems-1;
	else
	  currentItem += movement;
     }
   else
     {
	if( currentItem + movement <= 0 )
	  currentItem = 0;
	else
	  currentItem += movement;
     }
   
   /* Then check if we need to scroll */
   if( currentItem >= highest_item_on_list + height-2)
     {
	/* Check if we move just one step */
	if( movement == 1)
	  {
	     if( highest_item_on_list + (height-2)/2  > maxItems )
	       highest_item_on_list = maxItems - (height-2)/2;
	     else
	       highest_item_on_list += (height-2)/2;
	  }
	else
	  highest_item_on_list += movement;

	werase( win );
     }
   else if( currentItem < highest_item_on_list )
     {
	/* Check if we move just one step */
	if( movement == -1)
	  {
	     if( highest_item_on_list - (height-2)/2  < 0 )
	       highest_item_on_list = 0;
	     else
	       highest_item_on_list -= (height-2)/2;
	  }
	else
	  {
	     if( highest_item_on_list + movement < 0 )
	       highest_item_on_list = 0;
	     else
	       highest_item_on_list += movement;
	  }

	werase( win );
     }
	
   /* Redraw this panel */
   return( Display() );
}



