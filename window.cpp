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

#include "window.h"
#include "linklist.h"
#include "main.h"
#include "tools.h"

extern listelement *listpointer[];
extern struct    tablewin table[];


/* Constructor for the base-class 'pwindow' */
window::window( void )
{
}


/* Destructor for the window */
window::~window( void )
{
   #ifdef DEBUG_TO_TERM
     TermWrite( "Destructing window\n" );
   #endif

   delwin( win );
}


/* Set the window-title */
void window::SetTitle( char * lTitle )
{
   strcpy( title, lTitle );
}


/* Return the window-title */
char * window::GetTitle( void )
{
   return( title );
}


/* Move the window to position (x,y) */
void window::Move( int _x, int _y )
{
   x = _x;
   y = _y;
   mvwin( win, y, x );
//   wrefresh( win );
}


/* Resize the window to size (width, height) */
void window::Resize( int w, int h )
{
   width = w;
   height = h;
   wresize( win, height, width );
//   wrefresh( win );
}


/* Refresh the window, i.e. redraw it */
void window::Refresh( void )
{
/*   werase( win ); */
    wrefresh( win );
}


/* Erase the window */
void window::Erase( void )
{
   werase( win );
}


/* Return window x-position */
int window::X( void )
{
      return x;
}


/* Return window y-position */
int window::Y( void )
{
      return y;
}


/* Return window width */
int window::Width( void )
{
      return width;
}                               

/* Return window height */
int window::Height( void )
{
      return height;
}



/* Set the border-parts */
void window::SetBorder( chtype _ls, chtype _rs,
		                               chtype _ts, chtype _bs,
		                               chtype _tl, chtype _tr,
		                               chtype _bl, chtype _br)
{
      ls = _ls; rs = _rs;
      ts = _ts; bs = _bs;
      tl = _tl; tr = _tr;
      bl = _bl; br = _br;
}                             
