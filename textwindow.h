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

#ifndef TEXTWINDOW_H
#define TEXTWINDOW_H

#include "window.h"
#include "text.h"
#include "config.h"

#define WRAP   1
#define NOWRAP 0

class textwin : public window
{
 public:
   textwin::textwin( char *, char *, char *, int, bool );
   textwin::~textwin();
   
   void AddStr( char *, bool );
   void WriteLine( int, char * );
   void HandleEvents( void );
   void AddTextBuffer( char *, short );
   void TextAddTop( char *, ... );
   void TextAddBottom( char *, ... );
   void ShowText( int );
   
   
 private:
   void ScrollOneUp( void );
   void ScrollOneDown( void );
   void DrawHeader( void );
   void DrawFoot( void ); 
  int      top, left;
   int      topline;   /* The buffer-line at top */
   char     footer[64];
   
   char     **textbuffer;
   Text     * text;
   int      bufferlength, bufferend;
   int      insertPos;
};


#endif // TEXTWINDOW_H
