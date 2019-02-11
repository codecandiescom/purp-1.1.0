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

#ifndef PANEL_H
#define PANEL_H

#include "window.h"
#include "linklist.h"
#include "config.h"

#define INSTALLED 0
#define AVAILABLE 1

class panel : public window
{
   
 public:
   panel( short, char *, int, int, int, int );
   ~panel( void );
   
   void Refresh( void );
   void SetList( listelement * );
   void ScanFolder( void );

   listelement *MoveCursor( int );
   listelement *Display( void );

   int  GetCurrentItem( void );
   int  GetHighestItem( void );
   
   void SetCurrentItem( int );
   void SetHighestItem( int );

   void ChangeSelectedCount( int );
   void ChangeSelectedSize( unsigned long );
   
   int  GetSelectedCount( void );
   unsigned long GetSelectedSize( void );

 private:
   listelement   *list;
   short         panelID;
   int           selected_count;
   unsigned long selected_size;   
   int           highest_item_on_list;
   char          dirstring[128];
   int           maxItems;
   int           currentItem;

};



#endif  /* PANEL_H */

