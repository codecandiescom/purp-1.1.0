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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "textwindow.h"
#include "linklist.h"
#include "main.h"
#include "tools.h"
#include "text.h"

extern WINDOW    *win1, *win2, *infowin;
extern int        width, height, gMaxoptions[];
extern int        highest_item_on_list[];
extern char       dirstring[2][100];
extern int        panel;

extern listelement *listpointer[2];
extern int       highest_item_on_list[];
extern int       new_option[];


textwin::textwin( char *ltitle, 
		  char *lfooter, 
	          char *_text,
 		  int  bottommargin,
		  bool wrap )
{
   x = 0; 
   y = 1;
   
   top = 0;
   height = LINES - bottommargin - top;
   left = 0;
   width = COLS - left;
   
   strcpy( title, ltitle );
   strcpy( footer, lfooter );

   /* Create a textbuffer-object */
   text = new Text();
   
   /* Create a ncurses-window */
   win = newwin( height, width, top, left );
   if( win == NULL )
    {   
       fprintf( stderr, "Error: Unable to create text-window!\n" );
       endwin();
       exit(0);
    }
   
   wbkgd( win, COLOR_PAIR(1) );
   wattrset( win, COLOR_PAIR(1) );
   werase( win );   
   
   DrawHeader();
   DrawFoot();
   
   textbuffer = NULL;
   
   /* Add initial text */
   if( strlen( _text ) > 0 )
     AddStr( _text, wrap );
   
   /* At the beginning the first line are at top of the window */
   topline = 0;
   bufferlength = 0;
   insertPos = 0;
   
   wrefresh( win );
}

void textwin::DrawHeader( void )
{
   char tmpStr[66];
   wmove( win, top, left );
   whline( win, 0, width );

   if( strlen( title ) > 0 )
     {
	sprintf( tmpStr, " %s ", title );
	mvwaddch( win, top, left + 2, ACS_RTEE );
	mvwaddstr( win, top, left + 3, tmpStr );
	mvwaddch( win, top, strlen( tmpStr ) + left + 3, ACS_LTEE );
     }
}

void textwin::DrawFoot( void )
{
   char tmpStr[66]; int lX;
   wmove( win, top + height - 1, left );
   whline( win, 0, width );

   if( strlen( footer ) > 0 ) 
     {
	sprintf( tmpStr, " %s ", footer );
	lX = width - strlen( footer ) - 6;
	mvwaddch( win, top + height - 1, lX, ACS_RTEE );
	mvwaddstr( win, top + height - 1, lX + 1, tmpStr );
	mvwaddch( win, top + height - 1, left + width - 3, ACS_LTEE );
     }
}


/* Destructor for the window */
textwin::~textwin( void )
{
   int count;
   #ifdef DEBUG_TO_TERM
     TermWrite( "Destructing textwin\n" );
   #endif

   /* Destruct our text-item */
   delete text;
   
}



/* Add text to the textwindow */
void textwin::AddStr( char * text, bool wrap )
{
   char ch;
   char *tmpPtr = text;
   
   wmove( win, top + y, left + x );
   
   while( ((ch = *tmpPtr++) != 0) && ( y <= height - 2) )
     {
	if( ch == '\n' )
	  {
	     y++;
	     x = left;
	     wmove( win, top + y, left );
	  }
	else
	  {
	     waddch( win, ch );
	     x++;
	     if( x >= width )
	       {
		  if( wrap )
		    {
		       y++;
		       x = left;
		       if( y <= height - 2 )
			 wmove( win, y, x );
		    }
		  else
		    {
		       y++;
		       x = left;
		       tmpPtr = strchr( tmpPtr, '\n' ) + 1;	
		       if( y <= height - 2 )
			 wmove( win, y, x );
		    }
	       }
	  }
     }
   wrefresh( win );
}


/* Write text at specified line in the window.. */
void textwin::WriteLine( int line, char * text )
{
   char ch;
   char *tmpPtr = text;
   
   x = left;
   y = line;
   wmove( win, top + y, left + x );

   while( ((ch = *tmpPtr++) != 0) && (ch != '\n') && (x < width) )
     {
	waddch( win, ch );
 	x++;
     }
   
   wrefresh( win );
}


/* Scroll up, i.e. push text down and fill above */
void textwin::ScrollOneUp( void )
{
   topline--;
   wscrl( win, -1 );
   WriteLine( top + 1, text->GetLine( topline ) );
}

/* Scroll down, i.e. push text up and fill below */
void textwin::ScrollOneDown( void )
{
   topline++;
   wscrl( win, 1 );
   WriteLine( height-2, text->GetLine( topline + height - 3 ) );
}



/* Eat key-commands from the user.. */
void textwin::HandleEvents( )
{
   int key, count, lines;
   bool eventloop=1;
   int bufferlength = text->GetLength();
   wsetscrreg( win, top+1, height-2 );
   scrollok(win, TRUE);
   
   while( eventloop )
     {
	key = getch();

	switch(key)
	  {
	     
	   /* Up we go one line..*/  
	   case KEY_UP:
	   case '8':
             #ifdef DEBUG_TO_TERM
	         TermWrite( "User pressed 'up'\n" );
             #endif
	       if( topline > 0 )
	       {
		  ScrollOneUp();
		  wrefresh( win );
	       }
	   break;

	     
	   /* Down one line */
	   case KEY_DOWN:
	   case '2':
             #ifdef DEBUG_TO_TERM
	       TermWrite( "User pressed 'down'\n" );
             #endif
	     if( (topline + height - 2) < text->GetLength() )
	       {
		  ScrollOneDown();
		  wrefresh( win );
	       }
	     break;
	     
	     
	   /* Page up.. */
	   case KEY_PPAGE:
             #ifdef DEBUG_TO_TERM
	       TermWrite( "User pressed 'page-up'\n" );
             #endif
	     if( topline > 0 )
	       {
		  if( (topline - height - 2) < 0 )
		    topline = 0;
		  else
		    topline -= height-2;

		  werase( win );   
		  DrawHeader();
		  DrawFoot();
		  ShowText( topline );
		  wrefresh( win );
	       }
	     break;
	     
	     
	     /* Page down */
	   case KEY_NPAGE:
           #ifdef DEBUG_TO_TERM
	       TermWrite( "User pressed 'page-down'\n" );
           #endif

	     /* First we check that there is more to show below the bottom
	      * of the window. "topline + height - 2" is the bottom linenumber
	      * and if that is less than bufferlength there is more.
	      */
	     if( (topline + height - 2) < bufferlength )
	       {
		  if( (topline + (height-2)*2) < bufferlength )
		    topline += height-2;
		  else
		    topline += bufferlength - (topline + height - 3);
		  
		  werase( win );   
		  DrawHeader();
		  DrawFoot();
		  ShowText( topline );
		  wrefresh( win );
	       }
	     break;


           #ifdef DEBUG_TO_TERM
	     /* debug - key */
 	   case 'd':
	       TermWrite( "User pressed 'd' (dump debug-info)\n" );
	       text->Debug();
           break;
            #endif
	     
	     
	   default:
           #ifdef DEBUG_TO_TERM
	       TermWrite( "User pressed another key.. Remove textwindow\n" );
           #endif
	     eventloop = 0;
	  }
     }
}

/**************************************************************
 * textwin::ShowText
 * 
 * Display the text from text-list in the window 
 **************************************************************/

void textwin::ShowText( int line )
{
   int lines = height-3;
   char * t; int count = 0;

   if( line >= 0 && line < text->GetLength() )
     text->Goto( line );
   else
     text->Rewind();

   t = text->GetNext();
   while( count <= lines && t )
     {
	WriteLine( top+count+1, t );
	t = text->GetNext();
	count++;
     }
}



/**************************************************************
 * textwin::TextAddTop
 * 
 * Add text to the end of the internal Text-object
 **************************************************************/

void textwin::TextAddTop( char * _text, ... )
{
   char * textline;
   char tmpstr[1024] = ""; 
   char buf[ 1024 ];
   int bufptr;

   /* Create string by putting in arguments (like in printf) */
   va_list list;
   va_start( list, _text );
   int len = vsprintf( buf, _text, list );
   va_end( list );

   strcpy( tmpstr, buf );

   textline = strtok( tmpstr, "\n" );
   
   while( textline != NULL )
     {
	text->AddTop( textline );
	textline = strtok( NULL, "\n" );
     }
}

/**************************************************************
 * textwin::TextAddBottom
 * 
 * Add text to the end of the internal Text-object
 **************************************************************/

void textwin::TextAddBottom( char * _text, ... )
{
   char * textline;
   char tmpstr[1024] = ""; 
   char buf[1024];
   int bufptr;

   /* Create string by putting in arguments (like in printf) */
   va_list list;
   va_start( list, _text );
   int len = vsprintf( buf, _text, list );
   va_end( list );

   /* Cut the text to lines */
   strcpy( tmpstr, buf );
   textline = strtok( tmpstr, "\n" );
  
   while( textline != NULL )
     {
	text->AddBottom( textline );
	textline = strtok( NULL, "\n" );
     }
}

