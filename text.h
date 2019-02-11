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
   
#ifndef TEXT_H
#define TEXT_H

#include "config.h"

class TextLine
{
 public:
   TextLine( char * text );
   ~TextLine( void );

   TextLine * SetPrev( TextLine * line );
   TextLine * SetNext( TextLine * line );
   TextLine * GetNext( void );
   TextLine * GetPrev( void );
   
   char * GetText( void );
   void SetText( char * );
		
 protected:
   TextLine * next;
   TextLine * prev;
   char * text;
};


/* =============================================================== */
/*  Text -- A class representing a text of textlines               */
/* =============================================================== */

class Text
{
 public:
   Text( void );
   ~Text( void );

   void AddTop( char * text );          /* Add a line at the top */
   void AddBottom( char * text );       /* Add a line at the bottom */
   void AddAtCurrent( char * text );    /* Add a line at currentpoint */
   void AddAfterLine( char * text, 
		      long line );      /* Add a line after position 'line' */
   
   void Rewind( void );                 /* Set currentpoint to 0 */
   void Goto( long line );              /* Set currentpoint to 'line' */
   long GetCurrentpoint( void );        /* Return currentpoint */
   long GetLength( void );              /* Get length of text (lines) */
   char * GetLine( long line );         /* Get line with number 'line' */
   char * GetNext( void );              /* Get the next line in the text (at currentpoint) */
#ifdef DEBUG_TO_TERM
   void Debug( void );
#endif
   
 private:
   TextLine * first;
   TextLine * last;
   long lines; 
   long currentpoint;
};


#endif // TEXT_H
