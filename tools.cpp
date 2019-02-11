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
#include <unistd.h>
#include <curses.h>
#include <fcntl.h>
#include <ctype.h>

#include "main.h"
#include "linklist.h"
#include "mainloop.h"
#include "textwindow.h"
#include "tools.h"
#include "panel.h"

extern int termdev;
extern int tty[];
extern int currentPanel;


/****************************************************************************/
   
void * xmalloc( size_t size )
{
  register void *value = malloc( size );
  if( value == 0 )
     {
        #ifdef DEBUG_TO_TERM
	   TermWrite( "Error: xmalloc() Virtual memory exhausted! Tried to allocate %s bytes\n",
		     size );
        #endif
	  
	fatal( "virtual memory exhausted");
     }
   return value;
}

/****************************************************************************/

void fatal( char *ptr )
{
   printf("Fatal error: %s\n", ptr);
   exit(666);
}

/****************************************************************************/

#ifdef DEBUG_TO_TERM

/* A function that writes debug-messages to a textdevice.
 * Used for traceprint-debugging.
 */
int TermWrite( char *message, ...)
{
   va_list list;
   va_start( list, message );
   char buf[ 512 ];
   int bufptr;
   
   int len = vsprintf( buf, message, list );
   
   va_end( list );
   
   if( termdev != -1 )
     {
	write( termdev, buf, strlen( buf ) );
     }
   return strlen( buf );
}

#endif


/****************************************************************************/



void ErrorMessage( char * message, bool quit_purp )
{
   char tmpStr[256];
   
   if( quit_purp == FATAL )
     {
	sprintf( tmpStr, "\nPurp: %s (fatal error)\n", message );
	quitPurp( -4711, tmpStr );
     }
   else
     {
	sprintf( tmpStr, "\nPurp: %s\n", message );
     }
}

/****************************************************************************/
  
#ifdef DEBUG_TO_TERM

void DumpListTerm(listelement *lListpointer, short straight )
{
   listelement *localListPtr, *tmpPtr;
   localListPtr = lListpointer;

   while( localListPtr != NULL) 
     {     
	if( localListPtr->elementtype <= GROUP )   /* GROUP or DUMMY */
	  {
	     TermWrite( "%s/ (%d)\n", localListPtr->name, localListPtr->size );
	     if( localListPtr->elementtype == DUMMY )
	       if( localListPtr->prevlink )
		 TermWrite( "(sprev: %d - %s - %s)\n", localListPtr->prevlink, localListPtr->prevlink->name,
			   localListPtr->prevlink->link->name);
	  }
	else
	{
	    TermWrite( "%s %s-%s-%s  <>   %s\n", 
		     (localListPtr->is_marked == TRUE)?"M":"-",
		     localListPtr->name, 
		     localListPtr->version, 
		     localListPtr->release, localListPtr->filename );
	}
	
	
	if( straight == 1)
	  localListPtr = (listelement *)localListPtr -> straightlink;
	else
	  localListPtr = (listelement *)localListPtr -> link;
	  
     }
   if( localListPtr == NULL )
     TermWrite( "(NULL)\n" );
      
}
#endif


/****************************************************************************/

void LockTTY( short mode )
{
/*   int fd;

   if( mode == 1 )
     {
	tty[0] = dup(1);
	tty[1] = dup(2);
	
	fd = open("/dev/null", O_RDWR);
	dup2(fd, 1);
	dup2(fd, 2);
	
	if((fd != 1) && (fd != 2))
	  close(fd);
     }
   else
     {
	dup2( tty[0], 1);
	dup2( tty[1], 2);
	close( fd );
     }
*/
}
   


/**************************************************************
 * Convert 'string' to all lowercase and return it's pointer
 **************************************************************/

char * StrToLower( char * string )
{
   int i;
   for( i=0; i < strlen(string); i++ )
   {
	string[i] = tolower( string[i] );
   }

   return( string );
}



/**************************************************************
 * Checks if name ends in '.rpm'
 **************************************************************/
short IsRPM( char * name )
{
   int length = strlen( name );

   if( name[ length - 4 ] != '.' )
     return 0;
   if( name[ length - 3 ] != 'r' )
     return 0;
   if( name[ length - 2 ] != 'p' )
     return 0;
   if( name[ length - 1] != 'm' )
     return 0;

   return 1;
}
