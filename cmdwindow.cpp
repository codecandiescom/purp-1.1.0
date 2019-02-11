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

#include "main.h"
#include "linklist.h"
#include "tools.h"
#include "textwindow.h"
#include "cmdwindow.h"
#include "list.h"
#include "install.h"

extern listelement *listpointer[];
extern int       new_option[];
extern char      *availdir;
extern short     rpmdb_write_access;




cmdwindow::cmdwindow( int _y, int _width )
{
   x = 0; y = _y;
   width = _width;
   height = 1;
   
   win = newwin( height, width, y, x );
   if( win == NULL )
     ErrorMessage( "Unable to create progressbar-window!", FATAL );
   

   /* Set background-color */
   wbkgd( win, COLOR_PAIR(5) );
   
   /* Set colors */
   wattrset( win, COLOR_PAIR(5) );

   /* Erase window */
   werase( win );    
}



void cmdwindow::Draw( short bar )
{
   currentBar = bar;
   
   switch( bar )
     {
      case MAINLINE:
	if( rpmdb_write_access )
	  {
	     DrawCmdLine( MAIN_CMD_LINE );
             #ifdef DEBUG_TO_TERM
	       TermWrite( "cmdwin::Draw( MAIN_CMD_LINE )\n" );
             #endif
	  }
	else
	  {
	     DrawCmdLine( LIMITED_MAIN_CMD_LINE );
             #ifdef DEBUG_TO_TERM
	       TermWrite( "cmdwin::Draw( LIMITED_MAIN_CMD_LINE )\n" );
             #endif
	  }
	break;
      
      case LISTLINE:
	DrawCmdLine( LIST_CMD_LINE );	
        #ifdef DEBUG_TO_TERM
	  TermWrite( "cmdwin::Draw( LIST_CMD_LINE )\n" );
        #endif
	break;
      
      case CONFIGLINE:
	break;
      
      case CONFIRMLINE:
	DrawCmdLine( CONFIRM_CMD_LINE );
        #ifdef DEBUG_TO_TERM
	  TermWrite( "cmdwin::Draw( CONFIRM_CMD_LINE )\n" );
        #endif
	break;

      case UNINSTALLLINE:
	DrawCmdLine( UNINSTALL_CMD_LINE );
        #ifdef DEBUG_TO_TERM
	  TermWrite( "cmdwin::Draw( UNINSTALL_CMD_LINE )\n" );
        #endif
	break;

      case FORCELINE:
	DrawCmdLine( FORCE_CMD_LINE );
        #ifdef DEBUG_TO_TERM
	  TermWrite( "cmdwin::Draw( FORCE_CMD_LINE )\n" );
        #endif
	break;

      case ABOUTLINE:
	DrawCmdLine( ABOUT_CMD_LINE );
        #ifdef DEBUG_TO_TERM
	  TermWrite( "cmdwin::Draw( ABOUT_CMD_LINE )\n" );
        #endif
	break;

      case DEPENDENCIESLINE:
	DrawCmdLine( DEPENDENCIES_CMD_LINE );
        #ifdef DEBUG_TO_TERM
	  TermWrite( "cmdwin::Draw( DEPENDENCIES_CMD_LINE )\n" );
        #endif
	break;

      case SEARCHLINE:
	DrawCmdLine( SEARCH_CMD_LINE );
        #ifdef DEBUG_TO_TERM
	  TermWrite( "cmdwin::Draw( SEARCH_CMD_LINE )\n" );
        #endif
	break;
     }
}



void cmdwindow::Draw( void )
{
   Draw( currentBar );
}


/***************************************************************
 * Draw the cmdline passed as an argument..
 * 
 * Interprets the following control-codes:
 * 
 * '^'   - toggles bold text-mode
 * '_'   - toggles plain text-mode
 * 
 ***************************************************************/

void cmdwindow::DrawCmdLine( char * textline )
{
   char *txtPtr = textline;
   short count;

   short plain = 0;
   
#ifdef DEBUG_TO_TERM
   TermWrite( "cmdwin::DrawCmdLine\n" );
#endif
   
   wmove( win, 0, 0 );
   wattrset( win, COLOR_PAIR(1) );
   wattrset( win, A_NORMAL );
   werase( win );
   
   for( count = 0; count < strlen( textline ); count++ )
     {
	/* _ Toggles plain-mode (Non-bold) */
	if( *txtPtr == '_' )
	  {
	     if( plain == 0 )
	       {
		  plain = 1;
		  wattrset( win, COLOR_PAIR( 5 ) );
	       }
	     else
	       {
		  plain = 0;
		  wattrset( win, A_REVERSE + COLOR_PAIR( 5 ) );
	       }
	     txtPtr++;
	  }
	
	/* ^ Starts bold-mode (showing the hotkey) */
	else if( *txtPtr == '^')
	  {
	     wattrset( win, A_REVERSE + A_BOLD );
	     txtPtr++; count++;
	     waddch( win, *txtPtr );
	     wattrset( win, A_REVERSE + COLOR_PAIR( 5 ) );
	     txtPtr++;
	  }	
	
	/* Spaces in black.. */
	else if( *txtPtr == ' ' && !plain )
	  {
	     wattrset( win, A_NORMAL + COLOR_PAIR( 5 ));
	     waddch( win, ' ' );
	     wattrset( win, A_REVERSE + COLOR_PAIR( 5 ) );
	     txtPtr++;
	  }
	else
	  {
	     waddch( win, *txtPtr );
	     txtPtr++;	
	  }
     }

   wrefresh( win );   
}



/*************************************************************
 * HandleListLine
 * Handles the available selections for the ListLine
 *************************************************************/

void cmdwindow::HandleListLine( listelement *lListpointer, int panel, int option )
{
   int key;
   short loop = 1;
   
#ifdef DEBUG_TO_TERM
   TermWrite( "cmdwin::HandleListLine\n" );
#endif
   
   while( loop )
     {
	key = getch();
	switch(key)
	  {
	   case 'f':
	   case 'F':
	     
	     ListPackageFiles( lListpointer, panel, option );
	     
	     loop = 0;
	     break;
	     
	   case 's':
	   case 'S':

	     ListPackageScripts( lListpointer, panel, option );

	     loop = 0;
	     break;

	   case 'd':
	   case 'D':

	     ListPackageDependencies( lListpointer, panel, option );

	     loop = 0;
	     break;

	   case 'a':
	   case 'A':
	     loop = 0;
	     break;
	     
	   default:
	     beep();
	     wrefresh( win );
	     break;
	  }
     }
}


   
   
/*************************************************************
 * HandleConfirmLine
 * Gives the user the ability to confirm actions (Yes/no)
 *************************************************************/

bool cmdwindow::HandleConfirmLine( void )
{
   int key;
   short loop = 1;
   bool lResult;
   
#ifdef DEBUG_TO_TERM
   TermWrite( "cmdwin::HandleConfirmLine\n" );
#endif

   while( loop )
     {
	key = getch();
	switch(key)
	  {
	   case 'y':
	   case 'Y':
	     lResult = TRUE;
	     loop = 0;
	     
	     break;
	     
	   case 'n':
	   case 'N':
	     lResult = FALSE;
	     loop = 0;
	     break;
	     
	   default:
	     beep();
	     wrefresh( win );
	     break;
	  }
     }
   return lResult;
}



/*************************************************************
 * HandleForceLine
 * Ask if the user wants to force installation
 *************************************************************/

bool cmdwindow::HandleForceLine( void )
{
   int key;
   short loop = 1;
   bool lResult;
   
#ifdef DEBUG_TO_TERM
   TermWrite( "cmdwin::HandleForceLine\n" );
#endif

   while( loop )
     {
	key = getch();
	switch(key)
	  {
	   case 'f':
	   case 'F':
	     lResult = TRUE;
	     loop = 0;
	     
	     break;
	     
	   case 'a':
	   case 'A':
	     lResult = FALSE;
	     loop = 0;
	     break;
	     
	   default:
	     beep();
	     wrefresh( win );
	     break;
	  }
     }
   return lResult;
}



/*************************************************************
 *  * HandleDependenciesLine
 *  *************************************************************/

short cmdwindow::HandleDependenciesLine( void )
{
   int key;
   short loop = 1;
   short result;
   
#ifdef DEBUG_TO_TERM
   TermWrite( "cmdwin::HandleDependenciesLine\n" );
#endif
   
   while( loop )
     {
	key = getch();
	switch(key)
	  {
	   case 'f':
	   case 'F':
	     result = DEP_FORCE;
	     loop = 0;
	     
	     break;                 

	   case 'a':
	   case 'A':
	     result = DEP_ABORT;
	     loop = 0;
	     break;
	     
	   default:
	     beep();
	     wrefresh( win );
	     break;
	  }
     }
   return result;
}                           

/*************************************************************
 * HandleSearchLine
 *************************************************************/

void cmdwindow::HandleSearchLine( int panel, char *name )
{
   int key = 0;
   short loop = 1;
   char *nameptr = name;
   int namecount = 0;
   int xpos, ypos;

   #ifdef DEBUG_TO_TERM
      TermWrite( "cmdwin::HandleSearchLine\n" );
   #endif

   /* Read name from keyboard */
   while( (key != KEY_ENTER) && (key != 10) && (key != 13) )
     {
	key = getch();
	if( (key == 127) || (key == KEY_BACKSPACE) )  /* Backspace? */
	  {
	     if( namecount > 0 )
	       {
		  nameptr--;
		  namecount--;
		  getyx( win, ypos, xpos );
		  wmove( win, ypos, xpos-1 );
		  wdelch( win );
	       }
	  }
	else
	  {
	     *nameptr = key;
	     namecount++;
	     nameptr++;
	     waddch( win, key );
	  }
	wrefresh( win );   
     }

   /* Terminate the string */
   *--nameptr = '\0';
}
