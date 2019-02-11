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
#include <curses.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

#include "panel.h"
#include "linklist.h"
#include "dir.h"
#include "main.h"
#include "mainloop.h"
#include "tools.h"

#include "cmdwindow.h"
#include "pathwindow.h"
#include "infowindow.h"
#include "pcurses.h"

#include "text.h"

#include "config/confclass.h"

int oldstdout;
int oldstderr;

bool        quitloop=FALSE;

listelement *listpointer[2];
listelement *topListpointer[2];

const char * rootdir = "/";

int tty[2];
char *availdir;

short rpmdb_write_access = -1;

/* panels */
panel *packagePanel[2];
int termWidth;  int termHeight;
int panelWidth; int panelHeight; 
int currentPanel;

/* other windows */ 
cmdwindow   *cmdwin;
pathwindow  *pathwin;
infowindow  *infowin;
int openWindows = -1;    /* bit-flags for marking that windows are open */  
short showingMainInterface;   /* 1 if the main interface is displayed */

short curses_initiated = 0;

Conf * purpconf = NULL;         /* Config-class */

#ifdef DEBUG_TO_TERM
int termdev = -1;
short debug_initiated = 0;
#endif

bool showall = false;
bool dimmed = true;

//#####################################################################

int main(int argc, char **argv)
{
   char tmpstr[255];
   char *term_env;
   int errval;
   availdir = (char *)xmalloc( 512 );

   
   /* Install signal-handlers */
#ifndef DEBUG_TO_TERM
   signal( SIGSEGV, sigSEGVhandler );
   signal( SIGHUP, sigHUPhandler );
   signal( SIGTERM, sigTERMhandler );
#endif
 
   /* Window-resize is disabled because of a bug in the ncurses-release
    * from RedHat. */
   //   signal( SIGWINCH, sigWINCHhandler );  

   /* Interpret the arguments on the commandline */
   if( argc > 1 )
     {
	if( ScanCommandLine( argc, argv ) )
	  return( 0 );
     }
   else
     {
	strcpy( availdir, "" );
     }
   
   
#ifdef DEBUG_TO_TERM
   TermWrite( "\n" );
   TermWrite( "===================================================\n" );
   TermWrite( "%s\n", PURP_VERSION_STRING );
   TermWrite( "===================================================\n" );
   TermWrite( "\n" );
#endif

   

   /* Read config file to config-class */
   purpconf = new Conf();
   
   char *home = getenv( "HOME" );
   if( home )
     {
	char path[256];
	sprintf( path, "%s/.purprc", home );
	if( ! purpconf->LoadFile( path ) )
	  purpconf->LoadFile("/etc/purprc");
     }
   else
     {
	purpconf->LoadFile("/etc/purprc");
     }
   
   /* Check if 	"showall = true" is set in the config-file */
   if( ! strcmp( "true", purpconf->Search("showall", "false" )))
     {
	showall = true;
     }

   /* Check if         "dimmed = false" is set in the config-file */
   if( ! strcmp( "false", purpconf->Search("dimmed", "true" )))
     {
	dimmed = false;
     }
   
   
   /* Change $TERM so we get color */
   term_env = getenv( "TERM" );
   if( !strcmp( term_env, "xterm" ) )
     setenv( "TERM", "xterm-color", TRUE );
   
   /* Initiate curses */
   InitCurses();
   openWindows = 0;   /* No windows open */
   CreateWindows();
      
   /* Initiate RPM */
   InitRPM();
 
   /* Start with cursor at the left panel */
   currentPanel = INSTALLED;
      
   /* Initiate lists for the package-hierarchies */
   listpointer[ INSTALLED ] = NULL;
   listpointer[ AVAILABLE ] = NULL;

   /* Create dummys */
   listpointer[ INSTALLED ] = CreateDummy( listpointer[ INSTALLED ] );
   listpointer[ AVAILABLE ] = CreateDummy( listpointer[ AVAILABLE ] );
      
   topListpointer[ INSTALLED ] = listpointer[ INSTALLED ];
   topListpointer[ AVAILABLE ] = listpointer[ AVAILABLE ];

   listpointer[ INSTALLED ] = ReadInstalledRPM( listpointer[ INSTALLED ] );
   listpointer[ AVAILABLE ] = ReadAvailableRPM( listpointer[ AVAILABLE ] );

   /* Store handles to the lists in the panels */
   packagePanel[ INSTALLED ]->SetList( listpointer[ INSTALLED ] );
   packagePanel[ AVAILABLE ]->SetList( listpointer[ AVAILABLE ] );

   /* After the startup: Show the real command-line */
   cmdwin->Draw( MAINLINE );
     

   /* Display the contents of the panels */
   packagePanel[INSTALLED ]->Display();
   packagePanel[ AVAILABLE ]->Display();

   /* Show info about the first package in the info-window */
   if( listpointer[currentPanel]->link != NULL )
     {
	infowin->Display( listpointer[currentPanel]->link );
	pathwin->Display( listpointer[currentPanel]->link );
     }

   /* Indicate that we show the main interface */
   showingMainInterface = 1;
   
   /******************************************************************/
   /* The MAIN-LOOP that handles the major part of Purp's functions! */
   /******************************************************************/
   MainLoop( listpointer );
         
   
   /* Change $TERM to original value */
   setenv( "TERM", term_env, TRUE );

   /************************************************/
   /* Close everything, free the world, halleluja! */
   /************************************************/
   quitPurp( 0, "" );
}


/*************************************************/
/* Initate RPM..                                 */
/*************************************************/
void InitRPM( void )
{
   rpmdb db = NULL;  /* for the rpm-database */
   uid_t  user_id; 
   
   /* Read the RPM-configs */
   if( rpmReadConfigFiles( NULL, NULL ))
     ErrorMessage( "Could not read config-files!", FATAL );

   
   #ifdef DEBUG_TO_TERM
     TermWrite( "RPM config-files read successfully..\n" );
   #endif

            
   /* Turn off status-messages from rpmlib */
     rpmSetVerbosity( RPMMESS_FATALERROR );
//   rpmSetVerbosity( RPMMESS_VERBOSE );
     
   
   /* This part checks whether the user has write-access to the
    * RPM-database. If we can't open it with write-permission
    * then we try to open it for reading.
    */
   rpmdb_write_access = 1;
   
   /* Get UID for current user */
   user_id = getuid();
   
   
//   LockTTY( 1 );
   
   
   if( user_id == 0 )
     {
	/* Try to open the RPM-database with write-access */
	if( rpmdbOpen( rootdir, &db, O_RDWR | O_EXCL, 0644) ) 
	  {
	     LockTTY( 0 );
             #ifdef DEBUG_TO_TERM
	       TermWrite( "Couldn't open RPM-database with write-permission!\n" );
             #endif
	  
	     ErrorMessage( "User doesn't have write-permission for the RPM-database!", NOT_FATAL );
	     ErrorMessage( "Starting Purp in limited mode (for normal users).", NOT_FATAL );
	
	     /* The user can't write, open with read-access only */
	     if( rpmdbOpen( rootdir, &db, O_RDONLY, 0644) ) 
	       {
                   #ifdef DEBUG_TO_TERM
		    TermWrite( "Couldn't open RPM-database at all!\n" );
                   #endif
		    
		   fprintf( stderr, "\nPurp: could not open rpm-database for read only! (fatal error)\n" );
		   exit( 4711 );
	       }
	     rpmdb_write_access = 0;
	  }
     }
   else
     {
	/* user-id not '0', i.e. a normal user */
	if( rpmdbOpen( rootdir, &db, O_RDONLY, 0644) ) 
	  {
            #ifdef DEBUG_TO_TERM
	       TermWrite( "Couldn't open RPM-database for reading!\n" );
             #endif
	     fprintf( stderr, "\nPurp: could not open rpm-database for read only! (fatal error)\n" );
	     exit( 4711 );
	  }
	rpmdb_write_access = 0;
     }
   
//   LockTTY( 0 );

   #ifdef DEBUG_TO_TERM
     TermWrite( "RPM-database opened successfully!\n" );
   #endif
     
   /* Close it again.. */
   if( db ) rpmdbClose( db );  
     
}


/***************************************
 * purp_rpmdbOpen()
 * Open the RPM-database
 ***************************************/

short purp_rpmdbOpen( rpmdb *db )
{
   /* Suppress messages from rpmlib */
   LockTTY( 1 );

   if( rpmdb_write_access )
     {
	/* Open with writeaccess */
	if( rpmdbOpen( rootdir, db, O_RDWR | O_EXCL, 0644) ) 
	  {
	     LockTTY( 0 );
             #ifdef DEBUG_TO_TERM
	       TermWrite( "Couldn't open RPM-database with write-permission!\n" );
             #endif
	  
	     ErrorMessage( "Couldn't open RPM-database with write-permission!", NOT_FATAL );
	     
	     /* Set flag to make it unable to do things that requires write-permission */
	     rpmdb_write_access = 0;

	     return 1;
	  }
     }
   else
     {
	/* Open read-only */
	if( rpmdbOpen( rootdir, db, O_RDONLY, 0644) ) 
	  {
	    LockTTY( 0 );
            #ifdef DEBUG_TO_TERM
	       TermWrite( "Couldn't open RPM-database for reading\n" );
            #endif
	    fprintf( stderr, "\nPurp: could not open rpm-database for read only! (fatal error)\n" );
	    exit( 666 );
	  }
     }
   LockTTY( 0 );
   return 0;
}
   




/*************************************************/
/* Signal handlers                               */
/*************************************************/
void sigSEGVhandler( int sig )
{
   #ifdef DEBUG_TO_TERM
   if( debug_initiated )
     TermWrite( "%s\n", SIGSEGV_MESSAGE );
   #endif
   
   ErrorMessage( SIGSEGV_MESSAGE, FATAL );

   if( curses_initiated )
     EndCurses();
}

void sigHUPhandler( int sig )
{
   #ifdef DEBUG_TO_TERM
    TermWrite( "%s\n", SIGHUP_MESSAGE );
   #endif
   
   ErrorMessage( SIGHUP_MESSAGE, FATAL );
}

void sigTERMhandler( int sig )
{
   #ifdef DEBUG_TO_TERM
     TermWrite( "%s\n", SIGTERM_MESSAGE );
   #endif
   
   ErrorMessage( SIGTERM_MESSAGE, FATAL );
}


/***************************************************
 * Handler for SIGWINCH
 * 
 * Called when the window in which Purp runs is
 * resized. Resize the user interface.
 ***************************************************/
void sigWINCHhandler( int sig )
{
   listelement * tmpPtr;
   
   if( showingMainInterface == 1 )
     resizeInterface();
   else
     showingMainInterface = 2;  /* Indicate that we must resize the interface */

 }



/* The actual resize is done here */
void resizeInterface( void )
{
   WINDOW *alertwin;
     
   /* Restart curses to fetch new size */
   //endwin();
   //   initscr();
  
   struct winsize size;
 
   if (ioctl(fileno(stdout), TIOCGWINSZ, &size) == 0) 
     {
	resizeterm( size.ws_row, size.ws_col );
	wrefresh(stdscr);       /* Linux needs this */
//	show_all();
     }                                                               
   
   //resizeterm(-1,-1);
   //wrefresh(stdscr);
   //clear();
   
   /* Get new size */
   getmaxyx( stdscr, termHeight, termWidth );

   /* Calculate sizes */
   panelWidth = termWidth / 2;
   panelHeight = termHeight - 9;
   
   /* Move & Resize installed-panel */
   //packagePanel[ INSTALLED ]->Resize( panelWidth, panelHeight );
   //packagePanel[ INSTALLED ]->Erase();
   
   /* Move & Resize available-panel */
   //packagePanel[ AVAILABLE ]->Resize( termWidth - panelWidth, panelHeight );
   //packagePanel[ AVAILABLE ]->Move( panelWidth, 0 );
   //packagePanel[ AVAILABLE ]->Erase();
   
   /* Move & Resize pathwin */
//   pathwin->Resize( termWidth, 1 );
//   pathwin->Move( 0, packagePanel[ INSTALLED ]->Height() );
//   pathwin->Erase();        
   
   /* Move & Resize info-win */
//   infowin->Resize( termWidth, termHeight - pathwin->Y() - pathwin->Height() -1 );
//   infowin->Move( 0, pathwin->Y() + pathwin->Height() );
//   infowin->Erase();
   
   /* Move & Resize command-win */
//   cmdwin->Resize( termWidth, 1 );
//   cmdwin->Move( 0, infowin->Y() + infowin->Height() );
//   cmdwin->Erase();

//   HandleRedraw( listpointer );

   packagePanel[ INSTALLED ]->Refresh();
   packagePanel[ AVAILABLE ]->Refresh();
   
   showingMainInterface = 1;
   

#ifdef DEBUG_TO_TERM
   TermWrite( "Resize:  Width = %d  /  Height = %d\n", termWidth, termHeight );
#endif                                 
}


/* Check if the Purp-window was resized 
 * showingMainInterface == 0 : other than main interface displayed 
 * showingMainInterface == 1 : main interface displayed
 * showingMainInterface == 2 : window resize when main interface wasn't displayed 
 */
void CheckResize( void )
{
   if( showingMainInterface == 2 )
     resizeInterface();
   else
     showingMainInterface = 1;
}


/*************************************************/
/* Close everything, free everything, halleluja! */
/*************************************************/
void quitPurp( short quitresult, char * message )
{

   /* Close down curses */
   if( openWindows >= 0 )
     EndCurses();
   
   /* Destroy the linked lists containing package-info, and free their memory */
   EraseList( topListpointer[ INSTALLED ] );
   EraseList( topListpointer[ AVAILABLE ] );

   free( availdir );
   
   #ifdef DEBUG_TO_TERM
     if( debug_initiated )
     {
	TermWrite( "Quiting Purp.. Good bye dear user!\n" );
	close( termdev );
     }
   #endif

     if( strcmp( message, "" ) )
       fprintf( stderr, "%s\n", message );
   
   /* Not much left to do.. Hope the user enjoyed Purp.. */
   exit( quitresult );
}




/*************************************************
 * Scan and parse commandline
 *************************************************/

short ScanCommandLine( int argc, char ** argv )
{
   int count;
   short argConsumed = 0;
   short quitAfter = 0;
   
   if( argc > 1 )
     {
	for( count=1; count < argc; count++ )
	  {
	     if( argv[ count ][ 0 ] == '-' )
	       {
		  InterpretFlag( argv, &count, &quitAfter );
	       }
	     else 
	       {
		  if( count < argc )
		    {
		       sprintf( availdir, "%s/", argv[ count ] ); 
		       break;
		    }
		  else
		    strcpy( availdir, "" );
	       }
	  }
     }
   return( quitAfter );
}


/*************************************************
 * Interpret flags from command-line
 *************************************************/

void InterpretFlag( char ** argv, int * count, short * quitAfter )
{
   char tmpStr[256];

   if( ! strcmp( argv[ *count ], "--help" ) )
     {
	PrintUsageInfo();
	*quitAfter = 1;
     }
   else if( ! strcmp( argv[ *count ], "-a" ) || ! strcmp( argv[ *count ], "--all" ))
     {
	showall = true;
     }
   else if( ! strcmp( argv[ *count ], "--version" ) )
     {
	sprintf( tmpStr, "%s\n", PURP_VERSION_STRING );
	fprintf( stdout, tmpStr );
	*quitAfter = 1;
     }
   
   else if( ! strcmp( argv[ *count ], "--debug" ) )
     {
#ifdef DEBUG_TO_TERM
	termdev = open(  argv[ *count + 1 ], O_WRONLY );
#else
	fprintf( stdout, "This version of Purp isn't built with Debug-support.\n" );
	*quitAfter = 1;
#endif
	*count = *count + 1;
     }
   
   else
     {
	fprintf( stdout, "Unknown parameter: %s\n\n", argv[ *count ] );
	PrintUsageInfo();
	*quitAfter = 1;
     }
}



/*************************************************
 * Print user-info
 *************************************************/

void PrintUsageInfo( void )
{
   fprintf( stdout, "Usage: purp [--option]... [package-dir]\n\n" );
   fprintf( stdout, "where [package-dir] is a directory containing rpm-packages\n\n" );
   fprintf( stdout, "options are:\n" );
   fprintf( stdout, "--help              display this help and exit\n" );
   fprintf( stdout, "--version           display version information and exit\n" );
   fprintf( stdout, "--all | -a          display already installed packages in the right panel\n" );
#ifdef DEBUG_TO_TERM
   fprintf( stdout, "--debug <device>    device to which debuginfo is sent\n\n" );
#endif
}



