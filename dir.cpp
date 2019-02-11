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

#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curses.h>
#include <fcntl.h>

#include "main.h"
#include "linklist.h"
#include "tools.h"
#include "dir.h"
#include "panel.h"

#include "progresswindow.h"



extern int       currentPanel;
extern char      * rootdir;

extern listelement *listpointer[2];
extern listelement *topListpointer[2];
extern panel *packagePanel[];

extern char *availdir;


//#####################################################################

listelement *ReadInstalledRPM( listelement *lListpointer  )
{
   Header h; 
   rpmdb db = NULL;

   
#ifdef DEBUG_TO_TERM
   TermWrite( "Reading list of installed packages.. " );
#endif

   /* Open the RPM-database */
   purp_rpmdbOpen( &db );
   

#if RPM_VERSION <= 304   
   int offset = rpmdbFirstRecNum( db );
   while( offset ) 
     {
	h = rpmdbGetRecord( db, offset );
	if( ! h ) 
	  {
	     fprintf(stderr, "could not read database record!\n");
	     exit(1);   
	  }
	AddItem( lListpointer, h, INSTALLED, "" );
	headerFree(h);
	offset = rpmdbNextRecNum( db, offset );
     }                                    
#else   
   rpmdbMatchIterator dbiter = rpmdbInitIterator( db, RPMDBI_PACKAGES, NULL, 0 );
   if( ! dbiter )
   {
      #ifdef DEBUG_TO_TERM
          TermWrite( "Could not create RPM iterator!" );
      #endif
      fprintf(stderr, "Could not create RPM iterator!\n");
      exit(1);   
   }  

   while( 1 ) 
     {
       h = rpmdbNextIterator( dbiter );
       if( h == NULL ) break;

       if( ! h ) 
	 {
	   fprintf(stderr, "could not read database record!\n");
	   exit(1);   
	 }
       AddItem( lListpointer, h, INSTALLED, "" );
     }                                    
   /* Free the rpmdb-iterator */
   rpmdbFreeIterator( dbiter );
#endif
   
   /* Close the rpm-database again */
   if( db ) rpmdbClose( db );
   
#ifdef DEBUG_TO_TERM
   TermWrite( "Done!\n" );
#endif
   
   return lListpointer;
}

//#####################################################################

listelement *ReadAvailableRPM( listelement *lListpointer )
{
   DIR *theDir;
   struct dirent *lDirent;
   struct stat st;

   static FD_t fd = NULL;    
   int rc;         
   Header h;
   char *name, tmpStr[128];
   int_32 count, type;         
   int dirSize = 0;
   short check;
   int result;

#ifdef DEBUG_TO_TERM
   TermWrite( "Reading list of available packages.. " );
#endif
   /* Check if the user wrote a package-directory */
   if( strcmp( availdir, "" ) )
     {
	if( (theDir = opendir( availdir )))
	  {
	     if(( lDirent = readdir( theDir )))
	       {
		  lstat( lDirent->d_name, &st );
		  
		  while(( lDirent = readdir( theDir )))
		    {
		       dirSize++;
		    }		  
	       }
	
	     if( dirSize > 0 )
	       {
		  /* Create a progressbar-object */
		  progresswindow *progressbar;
		  sprintf( tmpStr, "Reading available packages (%d)", dirSize );
		  progressbar = new progresswindow( tmpStr, dirSize, 0 );
		  
		  
		  rewinddir( theDir );
		  
		  if(( lDirent = readdir( theDir )))
		    {
		       while(( lDirent = readdir( theDir )))
			 {
			    /* Increase the progressbar */
			    progressbar->Update( +1 );
			    
			    if( IsRPM( lDirent->d_name ) )
			      {
				 sprintf( tmpStr, "%s/%s", availdir, lDirent->d_name );
				 fd = fdOpen( tmpStr, O_RDONLY, 0);
				 if( fdFileno(fd) >= 0)
				   {
				      result = rpmReadPackageHeader(fd, &h, NULL, NULL, NULL); 
				      fdClose(fd); 
				      if( result == 0 )
					{
					   if( h )
					     {
						AddItem( lListpointer, h, AVAILABLE, lDirent->d_name );
//						lListpointer = AddItem( lListpointer, h, AVAILABLE, lDirent->d_name );
						headerFree(h);
					     }
					}
				      else
					{
                                             #ifdef DEBUG_TO_TERM
					       TermWrite( "ReadAvailableRPM: Couldn't read package-header!\n" );
                                             #endif
					}
				      
				   }
				 else
				   {
                                      #ifdef DEBUG_TO_TERM
				        TermWrite( "ReadAvailableRPM: Couldn't open file (fdOpen)!\n" );
                                      #endif
				   }
			      }
			    
			 }
		    }
		  closedir( theDir );
		  
		  /* Delete the progressbar-object */
		  delete progressbar;
	       }
	  }
	else
	  {
	     char tmpErr[256];
	     sprintf( tmpErr, "No such file or directory '%s'", availdir );
	     ErrorMessage( tmpErr, FATAL );
	  }

#ifdef DEBUG_TO_TERM
	TermWrite( "Done!\n" );
#endif

	return lListpointer;
     }
}

/*************************************************************
 * ChangeDir
 * Enter the selected folder..
 *************************************************************/
listelement * ChangeDir( listelement *lListpointer, 
			 int panel_to_read )
{
   listelement *choosen = ReturnItemNo( lListpointer, 
				        packagePanel[ panel_to_read ]->GetCurrentItem() );
   listelement *tmpPtr = lListpointer;
   
   if( choosen->elementtype == GROUP )
     {
	/* Entering directory */
	lListpointer = (listelement *)choosen->grouplink;

	/* Going UP in hierarchy */
	if( ! strcmp( choosen->name, "..") )
	  {	     
	     packagePanel[ panel_to_read ]->SetCurrentItem( lListpointer->selected_item );
	     packagePanel[ panel_to_read ]->SetHighestItem( lListpointer->top_item );

	  }
	else   /* Going DOWN */
	  {	
	     /* Store the item currently selected */
	     tmpPtr->selected_item = packagePanel[ panel_to_read ]->GetCurrentItem();
	     tmpPtr->top_item = packagePanel[ panel_to_read ]->GetHighestItem();

     	     /* Going DOWN in hierarchy */
	     tmpPtr = (listelement *) lListpointer;
	     
	     packagePanel[ panel_to_read ]->SetCurrentItem( GetFirstNotEmpty( lListpointer ) );
	     packagePanel[ panel_to_read ]->SetHighestItem( 0 );
	  }	  
     }
   else
     beep();
   return lListpointer;
}

//#####################################################################

int GetFirstNotEmpty( listelement *lListpointer )
{
   listelement *tmpPtr = lListpointer;
   int count = 0;
   
   /* Go to element after the dummy */
   if( tmpPtr->link != NULL )
     tmpPtr = (listelement *)tmpPtr->link;
   
   while( tmpPtr->elementtype == GROUP && (tmpPtr->size == 0) )
     {
	count++;
	tmpPtr = (listelement *)tmpPtr->link;
     }
   
   return( count );
}



/*************************************************************
 * FindPackageOffset
 * Search the RPM-database and returns the offset for
 * the package specified by the listelement-argument.
 *************************************************************/
int FindPackageOffset( listelement * tmpPtr )
{
   dbiIndexSet matches;
   int i;
   int_32 count, type;
   char * version, * release;
   rpmdb db = NULL;
   Header h;

   /* Open the RPM-database */
   purp_rpmdbOpen( &db );


#if RPM_VERSION <= 304   
   
   /* Find all packages that match the name */
   if( rpmdbFindPackage( db, tmpPtr->name, &matches ) )
     {
	fprintf(stderr, "Error: could not search the RPM-database!\n");
	return( -1 );
     }

   /* Search through matches for packages that also match version and release */
   for( i = 0; i < matches.count; i++ )
     {
	if( matches.recs[ i ].recOffset )
	  {
	     h = rpmdbGetRecord( db, matches.recs[ i ].recOffset );
	     if( ! h )
	       {
		  fprintf(stderr, "Error: could not read RPM-database record!\n");
		  dbiFreeIndexRecord( matches ); 		       
		  return( -1 );
	       }
	     
	     headerGetEntry( h, RPMTAG_VERSION, &type, (void **) &version, &count ); 
	     headerGetEntry( h, RPMTAG_RELEASE, &type, (void **) &release, &count ); 

	     /* Chech if version and release matches */
	     if( ( ! strcmp( tmpPtr->version, version ) ) && 
		 ( ! strcmp( tmpPtr->release, release ) ) )
	       {
		  /* We have a perfect match! Clean up and return its offset */
		  i = matches.recs[ i ].recOffset;
		  dbiFreeIndexRecord( matches ); 		       
		  headerFree( h );
		  return( i );
	       }
	  }
     }

   /* Package wasn't found, clean up and return.. :-( */
   dbiFreeIndexRecord( matches ); 		       
   headerFree( h );

#else   
   
   /* Create an iterator that searches for name */
   rpmdbMatchIterator dbiter = rpmdbInitIterator( db, RPMTAG_NAME, tmpPtr->name, 0 );

   /* Search through matches for packages that also match version and release */
   while( 1 )
     {
       h = rpmdbNextIterator( dbiter );
       if( h == NULL ) break;

       headerGetEntry( h, RPMTAG_VERSION, &type, (void **) &version, &count ); 
       headerGetEntry( h, RPMTAG_RELEASE, &type, (void **) &release, &count ); 

       /* Chech if version and release matches */
       if( ( ! strcmp( tmpPtr->version, version ) ) && 
	   ( ! strcmp( tmpPtr->release, release ) ) )
	 {
	   /* We have a perfect match! Clean up and return its offset */
	   i = rpmdbGetIteratorOffset( dbiter );
	   /* Free the rpmdb-iterator */
	   rpmdbFreeIterator( dbiter );
	   if(db) rpmdbClose( db );
	   return( i );
	 }
     }

   /* Package wasn't found, clean up and return.. :-( */

   /* Free the rpmdb-iterator */
   rpmdbFreeIterator( dbiter );

#endif
   
   if( db ) rpmdbClose( db );
   
   return( -2 );
}




