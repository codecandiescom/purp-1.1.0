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

#ifndef WINDOW_H
#define WINDOW_H

#include <curses.h>  
#include "config.h"

class window
{
 public:
   window( void );
   ~window( void );
   
   void Move( int, int );
   void Resize( int, int );

   void Refresh( void );
   void Erase( void );
   
   void SetBorder( chtype, chtype, chtype, chtype, chtype, chtype, chtype, chtype );    
   
   void SetTitle( char * );
   char * GetTitle( void );
  
   int X( void );
   int Y( void );
   int Width( void );
   int Height( void );    
   
 protected:
   WINDOW   *win;
   char     title[256];
   int      x, y, width, height;
   
   chtype ls, rs, ts, bs, tl, tr, bl, br; 
};



#endif  /* WINDOW_H */

