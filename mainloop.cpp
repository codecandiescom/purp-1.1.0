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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>

#include "linklist.h"
#include "install.h"
#include "dir.h"
#include "main.h"
#include "mainloop.h"
#include "textwindow.h"
#include "tools.h"

#include "panel.h"
#include "cmdwindow.h"
#include "progresswindow.h"
#include "infowindow.h"
#include "pathwindow.h"

extern bool      quitloop;
extern short     rpmdb_write_access;
extern int       currentPanel;

extern listelement *topListpointer[2];

extern panel *packagePanel[];
extern cmdwindow   *cmdwin;
extern pathwindow  *pathwin;
extern infowindow  *infowin;
extern short showingMainInterface;   /* 1 if the main interface is displayed */


void MainLoop( listelement *listpointer[2] )
{
   listelement *listPtr;
   int key, count;
   bool result;
   listelement *tmpPtr;
   

   /* Show both panels */
   //   EraseAll();
   packagePanel[ INSTALLED ]->Refresh();
   packagePanel[ AVAILABLE ]->Refresh();

   infowin->Refresh();


   /* Main-loop, waiting for commands.. */
   while(quitloop == FALSE)
     {
	noecho();
	keypad(stdscr,TRUE);
	raw();

	key = getch();
	
	switch(key)
	  {
	     /* ^L - Repaint panels */
	   case 12:
             #ifdef DEBUG_SEE_KEY_EVENTS
             #ifdef DEBUG_TO_TERM
	     TermWrite( "User pressed '^L' (Repaint panels)\n" );
             #endif
             #endif

	       HandleRedraw( listpointer );
	   break;
	   

	     /* Quit */
	   case 'q':
	   case 'Q':
             #ifdef DEBUG_SEE_KEY_EVENTS
             #ifdef DEBUG_TO_TERM
	         TermWrite( "User pressed 'Q' (Quit Purp)\n" );
             #endif
             #endif
	       HandleQuit();
	   break;
	   

	     /* Pressing enter, entering groups */
	   case 10:
	   case 13:
	   case KEY_ENTER:
	   case KEY_RIGHT:
	   case '6':
             #ifdef DEBUG_SEE_KEY_EVENTS
             #ifdef DEBUG_TO_TERM
  	        TermWrite( "User pressed 'Enter' (Enter folder)\n" );
	     #endif
             #endif
	       HandleEnter( listpointer );
	       break;
	   
	     
	     /* Pressing left key, moving up in hierarchy */
	   case KEY_LEFT:
	   case '4':
             #ifdef DEBUG_SEE_KEY_EVENTS
             #ifdef DEBUG_TO_TERM
  	        TermWrite( "User pressed 'left' (Move up in hierarchy)\n" );
	     #endif
             #endif
	       tmpPtr = listpointer[ currentPanel ]->link;
	       if( ! strcmp( tmpPtr->name, "..") )
		 {
		    packagePanel[ currentPanel ]->SetCurrentItem( 0 );
		    HandleEnter( listpointer );
		 }
	       break;

	     
	   /* Up we go.. */  
	   case KEY_UP:
	   case '8':
             #ifdef DEBUG_SEE_KEY_EVENTS
             #ifdef DEBUG_TO_TERM
	        TermWrite( "User pressed 'Arrow-up' (scroll up)\n" );
	     #endif
             #endif
	       HandleKeyUp( listpointer );
	       break;

	     
	   /* Down.. Down.. Down.. */
	   case KEY_DOWN:
	   case '2':
             #ifdef DEBUG_SEE_KEY_EVENTS
             #ifdef DEBUG_TO_TERM
	        TermWrite( "User pressed 'Arrow-down' (scroll down)\n" );
	     #endif
             #endif
	       HandleKeyDown( listpointer );
	     break;

 	     
	   /* Page up.. */
	   case KEY_PPAGE:
             #ifdef DEBUG_SEE_KEY_EVENTS
             #ifdef DEBUG_TO_TERM
	        TermWrite( "User pressed 'Page-up' (scroll up)\n" );
	     #endif
             #endif
	       HandleKeyPageUp( listpointer );
	     break;
	   
	   /* Page down */
	   case KEY_NPAGE:
             #ifdef DEBUG_SEE_KEY_EVENTS
             #ifdef DEBUG_TO_TERM
	        TermWrite( "User pressed 'Page-down' (scroll down)\n" );
	     #endif
             #endif
	       HandleKeyPageDown( listpointer );
	     break;


	   /* KEY_IC = Insert-key, which marks the package */
	   case KEY_IC:
	   case '-':
	   case '+':
	   case ' ':
             #ifdef DEBUG_SEE_KEY_EVENTS
             #ifdef DEBUG_TO_TERM
	        TermWrite( "User pressed 'Insert' (Mark selection)\n" );
	     #endif
             #endif
	       if( rpmdb_write_access )
		 HandleKeyInsert( listpointer );
	     break;
	
	     
	   /* Tab-key, switch panel */
	   case 0x09:
	   case 't':
	   case 'T':
             #ifdef DEBUG_SEE_KEY_EVENTS
             #ifdef DEBUG_TO_TERM
	        TermWrite( "User pressed 'tab' (Toggle panel)\n" );
             #endif
	     #endif
	       HandleKeySwitchPanel( listpointer );
	     break;


	   /* Show description for a package */
	   case 'd':
	   case 'D':
             #ifdef DEBUG_SEE_KEY_EVENTS
             #ifdef DEBUG_TO_TERM
	        TermWrite( "User pressed 'D' (Describe)\n" );
             #endif
             #endif
	       HandleDescribePackage( listpointer );
	     break;

	     
	   /* Show information about Purp */
	   case 'a':
	   case 'A':
             #ifdef DEBUG_SEE_KEY_EVENTS
             #ifdef DEBUG_TO_TERM
	        TermWrite( "User pressed 'A' (About Purp)\n" );
             #endif
             #endif
	       HandleAbout( listpointer );
	     break;
	     
	   /* List.. */
	   case 'f':
	   case 'F':
             #ifdef DEBUG_SEE_KEY_EVENTS
             #ifdef DEBUG_TO_TERM
	        TermWrite( "User pressed 'f' (follow)\n" );
	     #endif
             #endif
	       HandleKeyFollow( listpointer );
	     break;
	     
	   /* List.. */
	   case 'l':
	   case 'L':
             #ifdef DEBUG_SEE_KEY_EVENTS
             #ifdef DEBUG_TO_TERM
	        TermWrite( "User pressed 'L' (List)\n" );
             #endif
	     #endif
	       HandleListPackage( listpointer );
	     break;

	     /* Search.. */
	   case 's':
	   case 'S':
             #ifdef DEBUG_SEE_KEY_EVENTS
             #ifdef DEBUG_TO_TERM
	        TermWrite( "User pressed 'S' (Search)\n" );
             #endif
             #endif
	     HandleKeySearch( listpointer );
	     break;

	   /* Install a package */
	   case 'i':
	   case 'I':
             #ifdef DEBUG_SEE_KEY_EVENTS
             #ifdef DEBUG_TO_TERM
	        TermWrite( "User pressed 'I' (Install)\n" );
	     #endif
             #endif
	       if( rpmdb_write_access )
		 HandleKeyInstall( listpointer, INSTALL );
	     break;

	   /* Upgrade packages */
	   case 'u':
	   case 'U':
             #ifdef DEBUG_SEE_KEY_EVENTS
             #ifdef DEBUG_TO_TERM
	       TermWrite( "User pressed 'U' (Upgrade)\n" );
             #endif
             #endif
	       if( rpmdb_write_access )
		 HandleKeyInstall( listpointer, UPGRADE );
	     break;

	     
	   /* Kill packages.. */
	   case 'e':
	   case 'E':
             #ifdef DEBUG_SEE_KEY_EVENTS
             #ifdef DEBUG_TO_TERM
	        TermWrite( "User pressed 'E' (Erase package)\n" );
	     #endif
             #endif
	       if( rpmdb_write_access )
		 HandleKeyErase( listpointer );
	     break;

#ifdef DEBUG_TO_TERM
	   case 'x':
	   case 'X':
	     TermWrite( "User pressed 'X' (Dump debug info)\n" );
	     DumpListTerm( listpointer[ currentPanel ], 0 );
	     break;
#endif
	       
	   default:       
	     break;
	  }
     }
}


/**********************************************************
 * HandleQuit
 * 
 * Quit this marvellous program.. 
 **********************************************************/
void HandleQuit( void )
{
   cmdwin->Draw( CONFIRMLINE );
   if( cmdwin->HandleConfirmLine() == TRUE )
     {
	clear();	
	quitloop = TRUE;
     }
   else
     {
	cmdwin->Draw( MAINLINE );
     }
}


/**********************************************************
 * HandleEnter
 * 
 * Enter the currently selected directory..
 **********************************************************/
void HandleEnter( listelement *listpointer[2] )
{
   listelement * tmpPtr;
   
   listpointer[ currentPanel ] = ChangeDir( listpointer[ currentPanel ], 
					    currentPanel );
   
   packagePanel[ currentPanel ]->SetList( listpointer[ currentPanel ] );
   packagePanel[ currentPanel ]->Erase();
   packagePanel[ currentPanel ]->Display();

   tmpPtr = ReturnItemNo( listpointer[ currentPanel ], 
			  packagePanel[ currentPanel ]->GetCurrentItem() );
   infowin->Display( tmpPtr );
   pathwin->Display( tmpPtr );
}



/**********************************************************
 * HandleKeyUp
 * 
 * Move the selection-bar up.. Surprising, isn't it?
 **********************************************************/
void HandleKeyUp( listelement *listpointer[2] )
{
   listelement * current_item;
   current_item = packagePanel[ currentPanel ]->MoveCursor( -1 );
   if(current_item)
     {
	infowin->Display( current_item );
	pathwin->Display( current_item );
     }
}


/**********************************************************
 * HandleKeyDown
 * 
 * Move the selection-bar down..
 **********************************************************/
void HandleKeyDown( listelement *listpointer[2] )
{
   listelement * current_item;
   current_item = packagePanel[ currentPanel ]->MoveCursor( 1 );
   if(current_item)
     {
	infowin->Display( current_item );
	pathwin->Display( current_item );
     }
}



/**********************************************************
 * HandleKeyPageUp
 * 
 * Move the selection-bar one page up.. 
 **********************************************************/
void HandleKeyPageUp( listelement *listpointer[2] )
{
   listelement * current_item;
   current_item = packagePanel[ currentPanel ]->MoveCursor( - packagePanel[ currentPanel ]->Height()+2 );
   if(current_item)
     {
	infowin->Display( current_item );
	pathwin->Display( current_item );
     }
}



/**********************************************************
 * HandleKeyPageDown
 * 
 * Move the selection-bar one page down..
 **********************************************************/
void HandleKeyPageDown( listelement *listpointer[2] )
{
   listelement * current_item;
   current_item = packagePanel[ currentPanel ]->MoveCursor( packagePanel[ currentPanel ]->Height()-2 );
   if(current_item)
     {
	infowin->Display( current_item );
	pathwin->Display( current_item );
     }
}



/**********************************************************
 * HandleKeyInsert
 * 
 * Toggle the mark-flag for the currently selected
 * package..
 **********************************************************/
void HandleKeyInsert( listelement *listpointer[2] )
{
   listelement * current_item;
   if( ToggleMark( listpointer[currentPanel], 
		   packagePanel[ currentPanel ]->GetCurrentItem() ) )
     {
	current_item = packagePanel[ currentPanel ]->MoveCursor( 1 );
	if(current_item) infowin->Display( current_item );
     }
}



/**********************************************************
 * HandleRedraw
 * 
 * Redraw all windows
 **********************************************************/
void HandleRedraw( listelement *listpointer[2] )
{
   listelement *tmpPtr = ReturnItemNo( listpointer[ currentPanel ], 
			packagePanel[ currentPanel ]->GetCurrentItem() );
   
   erase();
   refresh();

   infowin->Display( tmpPtr );
   pathwin->Display( tmpPtr );

   cmdwin->Draw();
   
   packagePanel[ INSTALLED ]->Erase();
   packagePanel[ AVAILABLE ]->Erase();
   packagePanel[ INSTALLED ]->Display();
   packagePanel[ AVAILABLE ]->Display();
   
}


/**********************************************************
 * HandleDescribePackage
 * 
 * Show description for the currently selected package..
 **********************************************************/
void HandleDescribePackage( listelement *listpointer[2] )
{
   char tmpstr[128];
   listelement *tmpPtr;

   tmpPtr = ReturnItemNo( listpointer[ currentPanel ], 
			  packagePanel[currentPanel]->GetCurrentItem() );
   
   if( tmpPtr->elementtype == PACKAGE )
     {
	showingMainInterface = 0;
	
	sprintf( tmpstr, "Info for: %s-%s-%s", tmpPtr->name, tmpPtr->version, tmpPtr->release );
	
	textwin *describewin;
	describewin = new textwin( tmpstr, 
				  "Press any key to continue..", 
				  tmpPtr->description, 0, WRAP );
	
	getch();
	delete describewin;
	
	packagePanel[ INSTALLED ]->Display();
	packagePanel[ AVAILABLE ]->Display();
	infowin->Refresh();
	pathwin->Refresh();
	cmdwin->Draw( MAINLINE );

        CheckResize();
     }
}



/**********************************************************
 * HandleAbout
 * 
 * Show information about Purp
 **********************************************************/
void HandleAbout( listelement *listpointer[2] )
{
   textwin *aboutWin;
   showingMainInterface = 0;
   
   aboutWin = new textwin( "Information about Purp", 
			  "Use arrow-keys to navigate, any other to continue.",
			  "", 0, WRAP );
	
   aboutWin->TextAddBottom( PURP_DESCRIPTION_STRING );
   aboutWin->TextAddBottom( "%s\n ", PURP_COPYRIGHT_STRING );

   aboutWin->TextAddBottom( " License information" );
   aboutWin->TextAddBottom( " -------------------" );
   aboutWin->TextAddBottom( " This program is free software; you can redistribute it and/or modify " );
   aboutWin->TextAddBottom( " it under the terms of the GNU General Public License as published by " );
   aboutWin->TextAddBottom( " the Free Software Foundation; either version 2 of the License, or" );
   aboutWin->TextAddBottom( " (at your option) any later version." );
   aboutWin->TextAddBottom( " " );
   aboutWin->TextAddBottom( " This program is distributed in the hope that it will be useful," );
   aboutWin->TextAddBottom( " but WITHOUT ANY WARRANTY; without even the implied warranty of" );
   aboutWin->TextAddBottom( " MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the" );
   aboutWin->TextAddBottom( " GNU General Public License for more details." );
   aboutWin->TextAddBottom( " " );
   aboutWin->TextAddBottom( " You should have received a copy of the GNU General Public License" );
   aboutWin->TextAddBottom( " along with this program; if not, write to the Free Software" );
   aboutWin->TextAddBottom( " Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA." );
   aboutWin->TextAddBottom( " " );
   aboutWin->TextAddBottom( " How do I use Purp?" );
   aboutWin->TextAddBottom( " ------------------" );
   aboutWin->TextAddBottom( " You should read the man-file or the README-file that was distributed" );
   aboutWin->TextAddBottom( " together with Purp. These contain the most acurate help and information." );
   aboutWin->TextAddBottom( " " );
   aboutWin->TextAddBottom( " Obtaining the latest version" );
   aboutWin->TextAddBottom( " ----------------------------" );
   aboutWin->TextAddBottom( " For latest information and version of Purp, look at the Purp homepage:" );
   aboutWin->TextAddBottom( " <http://www.lysator.liu.se/purp/>" );
   aboutWin->TextAddBottom( " " );
   aboutWin->TextAddBottom( " Contact the author" );
   aboutWin->TextAddBottom( " ------------------" );
   aboutWin->TextAddBottom( " Please send comments and bug-reports to: <pugo@pugo.org>" );
   aboutWin->TextAddBottom( " " );
   aboutWin->TextAddBottom( " Contribute to Purp!" );
   aboutWin->TextAddBottom( " -------------------" );
   aboutWin->TextAddBottom( " You are very welcome if you want to contribute to Purp! Purp can be" );
   aboutWin->TextAddBottom( " much better but I don't have time to do everything myself. If you" );
   aboutWin->TextAddBottom( " want to help developing Purp, please contact Anders Karlsson," );
   aboutWin->TextAddBottom( " <pugo@pugo.org>." );
   aboutWin->TextAddBottom( " " );
   aboutWin->TextAddBottom( " Current contributors are except from myself mainly Ingo Brueckl" );
   aboutWin->TextAddBottom( " which has sent several nice patches for the interface." );
   aboutWin->TextAddBottom( " " );

   aboutWin->ShowText(0);
   aboutWin->HandleEvents();
   delete aboutWin;
	
   packagePanel[ INSTALLED ]->Display();
   packagePanel[ AVAILABLE ]->Display();
   infowin->Refresh();
   pathwin->Refresh();
   cmdwin->Draw( MAINLINE );

   CheckResize();
}


/**********************************************************
 * HandleListPackage
 * 
 * Show info about the currently selected package..
 **********************************************************/
void HandleListPackage( listelement *listpointer[2] )
{
   listelement *tmpPtr;
   
   tmpPtr = ReturnItemNo( listpointer[ currentPanel ], 
			  packagePanel[ currentPanel ]->GetCurrentItem() );

   if( tmpPtr->elementtype == PACKAGE )
     {
	cmdwin->Draw( LISTLINE );
	cmdwin->HandleListLine( listpointer[currentPanel], currentPanel, 
			        packagePanel[ currentPanel ]->GetCurrentItem() );
	CheckResize();
	packagePanel[ INSTALLED ]->Display();
	packagePanel[ AVAILABLE ]->Display();
	infowin->Refresh();
	pathwin->Refresh();
	cmdwin->Draw( MAINLINE );
     }
}

/**********************************************************
 * HandleKeyFollow
 * 
 * Moves the panel not active so it shows the same
 * folder as the selected (if possible).
 **********************************************************/
void HandleKeyFollow( listelement *listpointer[2] )
{
   char tmpStr[256] = ""; 
   char *strPtr; 
   char *groupname;
   short found_folder = 1;
   int   other_panel = (currentPanel == 1 ? 0 : 1);

   listelement *selectedPtr, *groupPtr;;
   listelement *listPtr = topListpointer[ other_panel ];
   
   /* Copy group-path */
   strcpy( tmpStr, listpointer[currentPanel]->group );
   
   
   /* Initiate strtok, get first path-name */
   groupname = strtok( tmpStr, "/" );
   
   /* Traverse down in hierarchy and search for folder */
   while( groupname != NULL )
     {
	/* Break if the group doesn't exist */
	if( (groupPtr = FindGroup( listPtr, groupname )) == NULL )
	  {
	     found_folder = 0;
	     break;
	  }
	
	/* Enter the group */
	listPtr = (listelement *)groupPtr->grouplink;
	
	groupname = strtok( NULL, "/" );
   }

   
   /* Go to wanted folder if it did exist */
   if( found_folder )
     {
	listpointer[ other_panel ] = listPtr;
	
	packagePanel[ other_panel ]->SetList( listPtr );
	packagePanel[ other_panel ]->SetCurrentItem( 0 );
	packagePanel[ other_panel ]->SetHighestItem( 0 );	

	/* Show results */
	packagePanel[ other_panel ]->Erase();
	packagePanel[ other_panel ]->Display();
     }
}


/**********************************************************
 * HandleKeySearch
 * 
 * Search for a package
 **********************************************************/
void HandleKeySearch( listelement *listpointer[2] )
{
   listelement * found_package;
   char name[256];
   int count = 0;
   int height;
   
   cmdwin->Draw( SEARCHLINE );
   cmdwin->HandleSearchLine( currentPanel, name );

   if( strlen( name ) > 0 )
     {
	found_package = CheckPackageExists( currentPanel, name, NULL, NULL, CASE_INSENSITIVE );
	
	if( found_package->name )
	  {
	     infowin->Display( found_package );
	     
	     /* Go up to dummy */
	     while( found_package->elementtype != DUMMY )
	       {
		  count++;
		  found_package = (listelement *)found_package->prevlink;
	       }
	     
	     listpointer[ currentPanel ] = found_package;
	     packagePanel[ currentPanel ]->SetList( found_package );
	     packagePanel[ currentPanel ]->SetCurrentItem( count - 1 );
	     
	     
	     height = packagePanel[ currentPanel ]->Height() - 2; 
	     
	     /* Highest item on the list */
	     if( count <= height )
	       packagePanel[ currentPanel ]->SetHighestItem( 0 );	
	     else
	       packagePanel[ currentPanel ]->SetHighestItem( count - height / 2 );
	     
	     
	     /* Show results */
	     packagePanel[ currentPanel ]->Erase();
	     packagePanel[ currentPanel ]->Display();
	     
	     found_package = ReturnItemNo( listpointer[ currentPanel ], 
					  packagePanel[ currentPanel ]->GetCurrentItem() );
	     
	     infowin->Display( found_package );
	     pathwin->Display( found_package );
	     
	  }
     }
   cmdwin->Draw( MAINLINE );
}



/**********************************************************
 * HandleKeySwitchPanel
 * 
 * Toggles selected panel
 **********************************************************/
void HandleKeySwitchPanel( listelement *listpointer[2] )
{
   char tmpStr[255];
   int tmp_panel = (currentPanel == 0) ? 1 : 0;
   listelement *tmpPtr;
   
   if( topListpointer[ tmp_panel ]->straightlink != NULL )
     {
	currentPanel = tmp_panel;
	packagePanel[ AVAILABLE ]->Display();
	packagePanel[ INSTALLED ]->Display();
     }
   
   tmpPtr = ReturnItemNo( listpointer[ currentPanel ], 
			  packagePanel[currentPanel]->GetCurrentItem() );
   infowin->Display( tmpPtr );
   pathwin->Display( tmpPtr );
}



/**********************************************************
 * HandleKeyInstall
 * 
 * Installs or upgrades a package (handles both, selected
 * by setting operation to INSTALL or UPGRADE)
 **********************************************************/
void HandleKeyInstall( listelement *listpointer[2], short operation )
{
   listelement *tmpPtr;
   short result;
   
   if( topListpointer[AVAILABLE]->straightlink != NULL )
     {
	showingMainInterface = 0;
	
	if( packagePanel[AVAILABLE]->GetSelectedCount() == 0 )
	  {
	     /* We come here if the user pressed install/upgrade without
	      * having any packages marked. If the cursor are into the
	      * available-panel and over a package, then try to operate
	      * on that package */
	     if( currentPanel == AVAILABLE )
	       {
		  tmpPtr = ReturnItemNo( listpointer[ AVAILABLE ], 
					packagePanel[AVAILABLE]->GetCurrentItem() );
		  
		  if( tmpPtr->elementtype == PACKAGE && tmpPtr->installdate == (time_t) 0)
		    {
		       tmpPtr->is_marked = TRUE;
		       packagePanel[ currentPanel ]->ChangeSelectedSize( tmpPtr->size );
		       
		       result = InstallPackages( operation, INSTALL_TEMPORARY );
		       
		       if( result )
			 {
			    /* Something went wrong.. */
			    tmpPtr->is_marked = FALSE;
			 }
		       
		       packagePanel[ currentPanel ]->ChangeSelectedSize( 0 );
		    }
	       }
	  }
	else
	  {
	     InstallPackages( operation, INSTALL_MARKED );
	  }
	
	tmpPtr = ReturnItemNo( listpointer[ currentPanel ], 
			      packagePanel[ currentPanel ]->GetCurrentItem() );
	
	infowin->Display( tmpPtr );
	pathwin->Display( tmpPtr );

	cmdwin->Draw( MAINLINE );
	packagePanel[ AVAILABLE ]->SetList( listpointer[ AVAILABLE ] );
	packagePanel[ INSTALLED ]->SetList( listpointer[ INSTALLED ] );
	packagePanel[ AVAILABLE ]->ScanFolder();
	packagePanel[ INSTALLED ]->ScanFolder();
	packagePanel[ INSTALLED ]->Erase();
	packagePanel[ AVAILABLE ]->Erase();
	packagePanel[ INSTALLED ]->Display();
	packagePanel[ AVAILABLE ]->Display();

     	CheckResize();
     }
}
   


/**********************************************************
 * HandleKeyErase
 * 
 * Handles erase-events..
 **********************************************************/
void HandleKeyErase( listelement *listpointer[2] )
{
   listelement *tmpPtr;
   short installTemporary = 0;
   short result;
   
   showingMainInterface = 0;

   if( packagePanel[ INSTALLED ]->GetSelectedCount() == 0 )
     {
	/* We come here if the user pressed erase without
	 * having any packages marked. If the cursor are into the
	 * installed-panel and over a package, then try to operate
	 * on that package */
	if( currentPanel == INSTALLED )
	  {
	     tmpPtr = ReturnItemNo( listpointer[ INSTALLED ], 
				   packagePanel[ INSTALLED ]->GetCurrentItem() );
	     
	     if( tmpPtr->elementtype == PACKAGE )
	       {
		  tmpPtr->is_marked = TRUE;
		  result = InstallPackages( UNINSTALL, INSTALL_TEMPORARY );
		  
		  if( result )
		    {
		       /* Something went wrong.. */
		       tmpPtr->is_marked = FALSE;
		    }
	       }
	  }
     }
   else
     InstallPackages( UNINSTALL, INSTALL_MARKED );

   tmpPtr = ReturnItemNo( listpointer[ currentPanel ], 
			 packagePanel[ currentPanel ]->GetCurrentItem() );

   infowin->Display( tmpPtr );
   pathwin->Display( tmpPtr );
   
   cmdwin->Draw( MAINLINE );
   packagePanel[ AVAILABLE ]->SetList( listpointer[ AVAILABLE ] );
   packagePanel[ INSTALLED ]->SetList( listpointer[ INSTALLED ] );
   packagePanel[ AVAILABLE ]->ScanFolder();
   packagePanel[ INSTALLED ]->ScanFolder();
   packagePanel[ INSTALLED ]->Erase();
   packagePanel[ AVAILABLE ]->Erase();
   packagePanel[ INSTALLED ]->Display();
   packagePanel[ AVAILABLE ]->Display();

   CheckResize();
}
