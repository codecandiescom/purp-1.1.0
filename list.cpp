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
#include <unistd.h>
#include <fcntl.h>

#include "main.h"
#include "linklist.h"
#include "dir.h"
#include "tools.h"

#include "panel.h"
#include "textwindow.h"
#include "cmdwindow.h"

extern listelement *listpointer[];
extern int new_option[];
extern char *availdir;
extern short showingMainInterface;   /* 1 if the main interface is displayed */


/****************************************************************************
 * ListPackageFile
 * 
 * Lists all files in the specified package in a scrollable window
 ****************************************************************************/

void ListPackageFiles( listelement *lListpointer, int panel, int option )
{
   listelement *tmpPtr;
   int i;
   Header h; 
   int_32 count, type;
   char tmpStr[ 256 ];
   char *name, * version, * release;
   char ** dirList, ** fileList;
   int_32 * dirIndexes;
   int_32 * fileSizeList;
   int isSource;
   short ok = 0;
   static FD_t fd;    
   rpmdb db = NULL;

#if RPM_VERSION >= 400
   rpmdbMatchIterator dbiter = NULL;
#endif
   
   /* Open the RPM-database */
   purp_rpmdbOpen( &db );

   showingMainInterface = 0;
   tmpPtr = ReturnItemNo( lListpointer, option );
   
   if( panel == INSTALLED )
     {
#if RPM_VERSION <= 304   
	int offset = FindPackageOffset( tmpPtr );
	if( offset >= 0 )
	  {
	     h = rpmdbGetRecord( db, offset );
	     ok = 1;
	  }
#else	
	/* Create an iterator that searches for name, version and release */
	dbiter = rpmdbInitIterator( db, RPMTAG_NAME, tmpPtr->name, 0 );
	rpmdbSetIteratorVersion( dbiter, tmpPtr->version );
	rpmdbSetIteratorRelease( dbiter, tmpPtr->release );
	/* Search through matches for packages that also match version and release */
	h = rpmdbNextIterator( dbiter );
	ok = 1;
#endif
     }
   else
     {
	if ((fd = fdOpen( tmpPtr->filename, O_RDONLY, 0)) >= 0) 
	  {
	     rpmReadPackageHeader(fd, &h, &isSource, NULL, NULL); 
	     fdClose(fd);   
	     ok = 1;
	  }
     }

   if( ok )
     {
	headerGetEntry( h, RPMTAG_NAME, &type, (void **) &name, &count ); 
	headerGetEntry( h, RPMTAG_VERSION, &type, (void **) &version, &count ); 
	headerGetEntry( h, RPMTAG_RELEASE, &type, (void **) &release, &count ); 
	
	sprintf( tmpStr, "Files in package %s-%s-%s", name, version, release );
	
	textwin *filesWin;
	filesWin = new textwin( tmpStr, "Use arrow-keys to navigate, any other to continue.", "", 0 , NOWRAP);

	
#ifdef RPM_VERSION_303
	
	if( headerGetEntry( h, RPMTAG_FILENAMES, &type, (void **) &fileList, &count ))
	  {
	     headerGetEntry(h, RPMTAG_FILESIZES, &type,
			    (void **) &fileSizeList, &count); 
	     
	     for( i = 0; i < count; i++ )
	       {
		  filesWin->TextAddBottom( "%10d  %s\n", fileSizeList[ i ], fileList[ i ] );
	       }
	  }
	
#else  /* RPM version 3.0.4 and up uses BASENAMES and DIRNAMES.. */
	if( headerGetEntry( h, RPMTAG_BASENAMES, &type, (void **) &fileList, &count ))
	  {
	     headerGetEntry(h, RPMTAG_DIRNAMES, NULL, (void **) &dirList, NULL);
	     headerGetEntry(h, RPMTAG_DIRINDEXES, NULL, (void **) &dirIndexes, NULL);
	     headerGetEntry(h, RPMTAG_FILESIZES, &type, (void **) &fileSizeList, &count); 
	     
	     for( i = 0; i < count; i++ )
	       {
		  filesWin->TextAddBottom( "%10d  %s%s\n", 
			  fileSizeList[ i ], dirList[dirIndexes[i]], fileList[ i ] );
	       }
	  }
	else
	  {
	     filesWin->AddStr( "(package contains no files)", NOWRAP );
	  }
#endif	 
	
	/* Free the RPM-header */
	headerFree( h ); 

	/* Tell window to display itself */
	filesWin->ShowText(0);

	filesWin->HandleEvents();
	delete filesWin;
	
     }
   
#if RPM_VERSION >= 400
	/* Free the rpmdb-iterator */
	if(dbiter) rpmdbFreeIterator( dbiter );
#endif
   
   if(db) rpmdbClose( db );
   CheckResize();
}


/****************************************************************************
 * ListPackageScripts
 * 
 * Lists all scripts in the specified package in a scrollable window
 ****************************************************************************/

void ListPackageScripts( listelement *lListpointer, int panel, int option )
{
   listelement *tmpPtr;
   int i;
   Header h; 
   int_32 count, type;
   char tmpStr[1024] = "";
   int isSource;
   FD_t fd;    
   short ok = 0;
   
   char *name, *version, *release;
   char *prein;
   char *postin;
   char *preun;
   char *postun;
   char *verify;

#if RPM_VERSION >= 400
   rpmdbMatchIterator dbiter = NULL;
#endif

   rpmdb db = NULL;
   
   /* Open the RPM-database */
   purp_rpmdbOpen( &db );


   showingMainInterface = 0;

   tmpPtr = ReturnItemNo( lListpointer, option );


   if( panel == INSTALLED )
     {
#if RPM_VERSION <= 304   
	int offset = FindPackageOffset( tmpPtr );

	if( offset >= 0 )
	  {
	     h = rpmdbGetRecord( db, offset );	     
	     ok = 1;
	  }
#else
	dbiter = NULL;
	/* Create an iterator that searches for name, version and release */
	dbiter = rpmdbInitIterator( db, RPMTAG_NAME, tmpPtr->name, 0 );
	rpmdbSetIteratorVersion( dbiter, tmpPtr->version );
	rpmdbSetIteratorRelease( dbiter, tmpPtr->release );
	/* Search through matches for packages that also match version and release */
	h = rpmdbNextIterator( dbiter );
	ok = 1;
#endif
     }
   else
     {
	if ((fd = fdOpen( tmpPtr->filename, O_RDONLY, 0)) >= 0) 
	  {
	     rpmReadPackageHeader(fd, &h, &isSource, NULL, NULL); 
	     fdClose(fd);   
	     ok = 1;
	  }
     }

   if( ok )
     {
	/* Get name, version and release for the package */
	headerGetEntry( h, RPMTAG_NAME, &type, (void **) &name, &count ); 
	headerGetEntry( h, RPMTAG_VERSION, &type, (void **) &version, &count ); 
	headerGetEntry( h, RPMTAG_RELEASE, &type, (void **) &release, &count ); 
   	headerGetEntry( h, RPMTAG_PREIN, &type, (void **) &prein, &count );
	headerGetEntry( h, RPMTAG_POSTIN, &type, (void **) &postin, &count );		
	headerGetEntry( h, RPMTAG_PREUN, &type, (void **) &preun, &count );
	headerGetEntry( h, RPMTAG_POSTUN, &type, (void **) &postun, &count );
	headerGetEntry( h, RPMTAG_VERIFYSCRIPT, &type, (void **) &verify, &count );

	/* Construct a header */
	sprintf( tmpStr, "Scripts in package %s-%s-%s", name, version, release );

	textwin *filesWin;
	filesWin = new textwin( tmpStr, "Use arrow-keys to navigate, any other to continue.", "", 0, NOWRAP );

	filesWin->TextAddBottom( "\n \nPre-install script:\n-------------------------\n" );
	filesWin->TextAddBottom( "%s\n", (prein == NULL) ? "(None)" : prein );

	filesWin->TextAddBottom( "\n \nPost-install script:\n--------------------------\n" );
	filesWin->TextAddBottom( "%s\n", (postin == NULL) ? "(None)" : postin );
	
	filesWin->TextAddBottom( "\n \nPre-uninstall script:\n---------------------------\n" );
	filesWin->TextAddBottom( "%s\n", (preun == NULL) ? "(None)" : preun );

	filesWin->TextAddBottom( "\n \nPost-uninstall script:\n----------------------------\n" );
	filesWin->TextAddBottom( "%s\n", (postun == NULL) ? "(None)" : postun );
	
	filesWin->TextAddBottom( "\n \nVerify script:\n----------------------------\n" );
	filesWin->TextAddBottom( "%s\n", (verify == NULL) ? "(None)" : verify );

	/* Free the RPM-header */
	headerFree( h );

	/* Tell window to display itself */
	filesWin->ShowText(0);
	
	/* Give control to the textwindow */
	filesWin->HandleEvents();

	/* Delete the file-window */
	delete filesWin;
	
     }
   
#if RPM_VERSION >= 400
	/* Free the rpmdb-iterator */
	if(dbiter) rpmdbFreeIterator( dbiter );
#endif

   if(db) rpmdbClose( db );
   CheckResize();
}


/****************************************************************************
 * ListPackageDependencies
 * 
 * Lists dependencies for the specified package in a scrollable window
 ****************************************************************************/

void ListPackageDependencies( listelement *lListpointer, int panel, int option )
{
   listelement *tmpPtr;
   int i;
   Header h; 
   int_32 count, type;
   char tmpStr[ 256 ];
   char *name, * version, * release;
   char **provides, **requires;
   int isSource;
   short ok[2] = {0,0};
   int lines[2], line;
   FD_t fd;    
   rpmdb db = NULL;

#if RPM_VERSION >= 400
   rpmdbMatchIterator dbiter = NULL;
#endif

   /* Open the RPM-database */
   purp_rpmdbOpen( &db );

   showingMainInterface = 0;

   tmpPtr = ReturnItemNo( lListpointer, option );

   if( panel == INSTALLED )
     {
#if RPM_VERSION <= 304   
	int offset = FindPackageOffset( tmpPtr );
	
	if( offset >= 0 )
	  {
	     h = rpmdbGetRecord( db, offset );
	     ok[0] = 1;
	  }
#else
	/* Create an iterator that searches for name, version and release */
	dbiter = rpmdbInitIterator( db, RPMTAG_NAME, tmpPtr->name, 0 );
	rpmdbSetIteratorVersion( dbiter, tmpPtr->version );
	rpmdbSetIteratorRelease( dbiter, tmpPtr->release );
	/* Search through matches for packages that also match version and release */
	h = rpmdbNextIterator( dbiter );
	ok[0] = 1;
#endif
     }
   else
     {
	if ((fd = fdOpen( tmpPtr->filename, O_RDONLY, 0)) >= 0) 
	  {
	     rpmReadPackageHeader(fd, &h, &isSource, NULL, NULL); 
	     fdClose(fd);   
	     ok[0] = 1;
	  }
     }

   if( ok[0] )
     {
	headerGetEntry( h, RPMTAG_NAME, &type, (void **) &name, &count ); 
	headerGetEntry( h, RPMTAG_VERSION, &type, (void **) &version, &count ); 
	headerGetEntry( h, RPMTAG_RELEASE, &type, (void **) &release, &count ); 

	ok[0] = headerGetEntry( h, RPMTAG_PROVIDES, &type, (void **) &provides, &count ); 
	  lines[0] = count;
	ok[1] = headerGetEntry( h, RPMTAG_REQUIRENAME, &type, (void **) &requires, &count ); 
	  lines[1] = count;

	sprintf( tmpStr, "Dependenices for package %s-%s-%s", name, version, release );
	
	textwin *depWin;
	depWin = new textwin( tmpStr, "Use arrow-keys to navigate, any other to continue.", "", 0 , NOWRAP);
	
	if( lines[0] == 0 ) lines[0]++;
	if( lines[1] == 0 ) lines[1]++;

	line = 0;
	       	
	depWin->TextAddBottom( "\n \nProvides:\n" );
	depWin->TextAddBottom( "----------------------------\n" );
	
		
	if( ok[0] && (lines[0] > 0) )
	  {
	     for( i = 0; i < lines[0]; i++ )
	       {
		  depWin->TextAddBottom( "%s\n", (provides[i] == NULL) 
					? "(None)\n" : provides[i] );
	       }
	  }
	else
	  {
	     depWin->TextAddBottom( "(none)\n" );
	  }
	

	depWin->TextAddBottom( "\n \nRequires:\n" );
	depWin->TextAddBottom( "----------------------------\n" );

	if( ok[1] && (lines[1] > 1) )
	  {
	     for( i = 0; i < lines[1]; i++ )
	       {
		  depWin->TextAddBottom( "%s\n", (requires[i] == NULL) 
					? "(None)\n" : requires[i]  );
	       }
	  }
	else
	  {
	     depWin->TextAddBottom( "(none)\n" );
	  }

	/* Free the RPM-header */
	headerFree( h );

	/* Tell window to display itself */
	depWin->ShowText(0);

	/* Give control to the textwindow */
	depWin->HandleEvents();

	headerFree( h ); 
	delete depWin;
     }

#if RPM_VERSION >= 400
	/* Free the rpmdb-iterator */
	if(dbiter) rpmdbFreeIterator( dbiter );
#endif
   if(db) rpmdbClose( db );
   CheckResize();
}

