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
   
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "text.h"
#include "tools.h"

/***************************************************************************/
/* Textline                                                                */
/***************************************************************************/

TextLine::TextLine( char * text )
{
   next = NULL;
   prev = NULL;
   
   this->text = (char * )xmalloc( strlen( text ) + 4 );
   strcpy( this->text, text );
}

TextLine::~TextLine( void )
{
   if( this->text != NULL )
     free( this->text );
}

TextLine * TextLine::SetPrev( TextLine * line )
{
   this->prev = line;
   return line;
}

TextLine * TextLine::SetNext( TextLine * line )
{
   this->next = line;
   return next;
}


TextLine * TextLine::GetNext( void )
{
   return next;
}

TextLine * TextLine::GetPrev( void )
{
   return prev;
}

char * TextLine::GetText( void )
{
   return text;
}

void TextLine::SetText( char * text )
{
   free( this->text );
   this->text = (char * )xmalloc( strlen( text ) + 16 );
   strcpy( this->text, text );
}



/***************************************************************************/
/* Text                                                                    */
/***************************************************************************/

Text::Text( void )
{
   first = NULL;
   last = NULL;
   lines = 0;
}

Text::~Text( void )
{
   if( lines > 0 )
     {
	TextLine * item = first;
	TextLine * next;
	
	while( item != NULL )
	  {
	     next = item->GetNext();
	     delete item;
	     item = next;
	  }
     }
}


/* Add a line at the top */
void Text::AddTop( char * text )
{
   if( first != NULL )
     {
	TextLine * oldfirst = first;
	first = first->SetPrev( new TextLine( text ) );
	first->SetNext( oldfirst );
     }
   else
     {
	first = last = new TextLine( text );
     }
   
   lines++;
}


/* Add a line at the bottom */
void Text::AddBottom( char * text )
{
   if( last != NULL )
     {
	last = last->SetNext( new TextLine( text ) );
     }
   else
     {
	first = last = new TextLine( text );
     }
   lines++;
}
  
/* Add a line at currentpoint */
void Text::AddAtCurrent( char * text )
{
}

/* Add a line after position 'line' */
void Text::AddAfterLine( char * text, long line )
{
}
   
/* Set currentpoint to 0 */
void Text::Rewind( void )
{
   currentpoint = 0;
}


/* Set currentpoint to 'line' */
void Text::Goto( long line )
{
   currentpoint = line;
}

/* Return currentpoint */
long Text::GetCurrentpoint( void )
{
   return currentpoint;
}

/* Return length of text (lines) */
long Text::GetLength()
{
   return lines;
}

/* Get line with number 'line' */
char * Text::GetLine( long line )
{
   if( line >= 0 && line < lines )
     {
	TextLine * current = first;
	for( int i=0; i < line; i++ )
	  current = current->GetNext();
	
	return current->GetText();
     }
   else
     return "<No such line>";
}

/* Get the next line in the text (at currentpoint) and add one to currenpoint */
char * Text::GetNext( void )
{
   if( currentpoint < lines )
     {
	TextLine * current = first;
	for( int i=0; i < currentpoint; i++ )
	  current = current->GetNext();
	
	currentpoint++;
	return current->GetText();
     }
   else
     return NULL;
}

#ifdef DEBUG_TO_TERM
void Text::Debug( void )
{
   TextLine * item = first;
	
   while( item != NULL )
     {
	TermWrite( "Debug--> %s\n", item->GetText() );
	item = item->GetNext();
     }
}
#endif
