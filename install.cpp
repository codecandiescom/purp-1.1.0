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
#include <time.h>

#include "main.h"
#include "linklist.h"
#include "dir.h"
#include "cmdwindow.h"
#include "textwindow.h"
#include "panel.h"
#include "tools.h"
#include "install.h"
#include "rpmprogress.h"

extern int       currentPanel;
extern char      *rootdir;
extern char      *availdir;

extern panel *packagePanel[];
extern listelement *listpointer[2];
extern listelement *topListpointer[2];

extern cmdwindow * cmdwin;  /* The command-line-object */ 

extern int termWidth;       /* Term width  */
extern int termHeight;      /* Term height */

rpmprogress *progressbar;
int progressCount = 0;
extern bool showall;


#if RPM_VERSION <= 304   
static void * showProgress(const Header h, const rpmCallbackType what,
			   const unsigned long amount,
			   const unsigned long total,
			   const void * pkgKey, void * data) 
#else
static void * showProgress(const void * arg, const rpmCallbackType what,
			   const unsigned long amount,
			   const unsigned long total,
			   const void * pkgKey, void * data) 
#endif
{
#if RPM_VERSION >= 400
   Header h = (Header) arg;
#endif
   
   int flags = (int) data;
   void * rc = NULL;
   const char * filename;
   static FD_t fd = NULL;
   int width;
 
   /* Package information */
   char * name;
   char * version;
   char * release;
   unsigned long * size;
   char tmpStr[ 256 ];
   
      
   filename = (const char *)pkgKey;

   switch (what) 
     {
      case RPMCALLBACK_INST_OPEN_FILE:
	fd = fdOpen(filename, O_RDONLY, 0);

	/* Reset progressbar (only used for installation & upgrade) */
	headerGetEntry(h, RPMTAG_NAME, NULL, (void **) &name, NULL);
	headerGetEntry(h, RPMTAG_VERSION, NULL, (void **) &version, NULL);
	headerGetEntry(h, RPMTAG_RELEASE, NULL, (void **) &release, NULL);
	headerGetEntry(h, RPMTAG_SIZE, NULL, (void **) &size, NULL);
	
	sprintf( tmpStr, "%s-%s-%s", name, version, release );
	progressbar->Next( tmpStr, *size );
	progressCount = 0;

	return fd;

	
      case RPMCALLBACK_INST_CLOSE_FILE:
        #ifdef DEBUG_TO_TERM
	  TermWrite("showProgress: RPMCALLBACK_INST_CLOSE_FILE, fd=%d\n", fd);
	#endif
	fdClose(fd);
	break;
      
      case RPMCALLBACK_INST_START:
	break;
      
      case RPMCALLBACK_INST_PROGRESS:

	width = progressbar->Width() - 4;
	
	if( ((int) ((((float) amount) / total) * width)) >= progressCount )
	{
	  progressCount++;
	  progressbar->Update( amount );
	}
	
	break;
	
      case RPMCALLBACK_TRANS_PROGRESS:
      case RPMCALLBACK_TRANS_START:
      case RPMCALLBACK_TRANS_STOP:
      case RPMCALLBACK_UNINST_PROGRESS:
      case RPMCALLBACK_UNINST_START:
      case RPMCALLBACK_UNINST_STOP:
	/* ignore */
	break;
     }
   
   return NULL;
}



/**************************************************************
 * InstallPackages
 * 
 * Install marked packages
 **************************************************************/

short InstallPackages( short operation, short installTemporary )
{
   listelement *tmpPtr, *tmpPtr2, *tmpPtr3;
   struct textwindow *twin;
   char tmpStr[ 256 ], pkgName[ 256 ];
   int dep_fd, major, rc;
   static FD_t fd = NULL;    
   char * netsharedPath = NULL;   
   short confirmYes = 0, stopInstall = 0, forceInstall = 0, op_panel, result;
   int numTmpPackages = 0, numBinaryPackages = 0, numFailed = 0;
   int uninstallFlags = 0, interfaceFlags = 0;
   int i, installFlags = 0; 
   int probFilter = 0;
 
   
   Header h = NULL;
   int offset;
   int isSource;      

   rpmTransactionSet rpmdep = NULL;
   rpmRelocation * relocations = NULL;
   rpmProblemSet probs = NULL;
   
   /* Dependencies-variables */
   struct rpmDependencyConflict * conflicts;
   int numConflicts;  

   rpmdb db = NULL;
   
   /* Open the RPM-database */
   purp_rpmdbOpen( &db );
   
   /* Choose which list to work on */
   if( operation == UNINSTALL )
     {
	tmpPtr = topListpointer[ INSTALLED ];
	op_panel = INSTALLED;
     }
   else
     {
	tmpPtr = topListpointer[ AVAILABLE ];
	op_panel = AVAILABLE;
     }
   
   
   #ifdef DEBUG_TO_TERM
   switch( operation )
     {
      case INSTALL:
	TermWrite( "InstallPackages()\n" );
	break;
      case UPGRADE:
	TermWrite( "UpgradePackages()\n" );     
	break;
      case UNINSTALL:
	TermWrite( "ErasePackages()\n" );     
	break;
     }
   #endif
     
   

   /* Create the confirm-window */
   textwin *listwin;
   switch( operation )
     {
      case INSTALL:
	listwin = new textwin( "Please confirm installation of following packages", "", "", 1, NOWRAP); 
	break;
      case UPGRADE:
	listwin = new textwin( "Please confirm upgrade of following packages", "", "", 1, NOWRAP); 
	break;
      case UNINSTALL:
	listwin = new textwin( "Please confirm uninstallation of following packages", "", "", 1, NOWRAP); 
	break;
     }
   
   /* Jump to element after the dummy */
   if( tmpPtr->straightlink != NULL ) 
     tmpPtr = (listelement *)tmpPtr->straightlink;
   
   /* Initiate dependencies-structure */
   rpmdep = rpmtransCreateSet( db, rootdir );  
   
   /* Show all marked files in the confirm-window */
   while( tmpPtr != NULL )
     {
	if( tmpPtr->is_marked )
	  {
	     if( operation <= UPGRADE )
	       {
		  /* open file to get header for dependencies-check */
		  fd = fdOpen( tmpPtr->filename, O_RDONLY, 0);
		  if( fdFileno(fd) >= 0)
		    {
		       /* Read the package header */
		       result = rpmReadPackageHeader( fd, &h, &isSource, NULL, NULL);
		       fdClose(fd);   
		       
		       /* Do we have a header? */
		       if( result == 0 )
			 {
			    /* If this isn't a uninstall, then go on.. */
			    if( operation <= UPGRADE )
			      {
				 /* Add dependencies information */
				 rc = rpmtransAddPackage(rpmdep, h, NULL, tmpPtr->filename,
						 operation-1, relocations);
				 
				 headerFree(h);
				 /* Error handling. I really should handle this better! */
				 if( rc ) 
				   {
				      if( rc == 1 )
					{
                                           #ifdef DEBUG_TO_TERM
					      TermWrite( "Error reading from file '%s'!\n", 
							tmpPtr->filename );
                                           #endif
					}
				      else if (rc == 2)
					{
                                           #ifdef DEBUG_TO_TERM
					      TermWrite( "Package '%s' needs a newer version of rpmlib!\n", 
							  tmpPtr->filename);
					   #endif
					} 
				   } /* if( rc ) */
				 
			      } /* if( operation <= UPGRADE ) */
			    			    
			    numBinaryPackages++;
			    
			 } /* if( result == 0 ) */
		       
		    } /* if( fdFileno(fd) >= 0) */
		  else
		    {
		       /* We couldn't open the file. I really should handle this better! */
                       #ifdef DEBUG_TO_TERM
		         TermWrite( "Error: fd = fdOpen( tmpPtr->filename, O_RDONLY, 0))\n");
                       #endif
		    }
	       }
	     else
	       {
		  /* REMOVE packages */
		  
		  offset = FindPackageOffset( tmpPtr );
		  rpmtransRemovePackage( rpmdep, offset );
	       }

	     sprintf( tmpStr, "%s/%s-%s-%s\n", 
		     tmpPtr->group, 
		     tmpPtr->name, 
		     tmpPtr->version, 
		     tmpPtr->release );
	     
	     listwin->AddStr( tmpStr, NOWRAP );
	     
	  }
	tmpPtr = (listelement *)tmpPtr->straightlink;
     }
   
   
   /* Confirm installation! */
   cmdwin->Draw( CONFIRMLINE );

   /* Check if the user pressed ok! */
   if( cmdwin->HandleConfirmLine() == TRUE )
     confirmYes = 1;

   /* Kill the list-window */
   delete listwin;


   /* The user confirmed! Time to do some business! */
   if( confirmYes )
     {
	/* Summarize the dependencies-check */
	if( rpmdepCheck( rpmdep, &conflicts, &numConflicts) )
	  {
             #ifdef DEBUG_TO_TERM
	       TermWrite("<---- Dependencies conflict! ---->\n");
             #endif

	     stopInstall = 1;
	  }
	
	/* User confirmation */
	if( ! stopInstall && conflicts )
	  {
             #ifdef DEBUG_TO_TERM
	       TermWrite("<---- Dependencies conflict! ---->\n");
             #endif

	     /* Ask user about dependency-problems */
	     result = HandleDependenyConflicts( operation, tmpPtr, conflicts, numConflicts );
	     rpmdepFreeConflicts( conflicts, numConflicts );  
	     if( result == DEP_FORCE ) 
	       forceInstall = 1;
	     
	     stopInstall = 1;
	  }

	/* Order the packages to satisfy the dependencies */
	if( rpmdepOrder(rpmdep) ) 
	  {
	     /* Oops error! */
	     /* Should add user interaction here! */
	     stopInstall = 1;
	  }


	if( ! stopInstall || forceInstall )
	  {
	
	     /* probFilter |= RPMPROB_FILTER_OLDPACKAGE; */
	   
	     progressCount = 0;
	     if( operation == INSTALL )
	       {
		  /* Create a progress-bar */
		  progressbar = new rpmprogress( "Installing", 
		  	            packagePanel[ AVAILABLE ]->GetSelectedSize(),
				    termHeight, termWidth );
	       }
	     else if( operation == UPGRADE )
	       {
		  /* Create a progress-bar */
		  progressbar = new rpmprogress( "Upgrading",
				    packagePanel[ AVAILABLE ]->GetSelectedSize(),
				    termHeight, termWidth );
	       }
		  
	     
	     /* Suppress messages from rpmlib */
//	     LockTTY( 1 );
	     
	     /* Actually perform our transactions! */
	     if( operation <= UPGRADE )
	       {
		  /* Set the operation to replace old packages and files */
		  probFilter |= RPMPROB_FILTER_REPLACEPKG |
		                RPMPROB_FILTER_REPLACEOLDFILES |
		                RPMPROB_FILTER_REPLACENEWFILES |
		                RPMPROB_FILTER_OLDPACKAGE;

		  /* Really do the RPM-management! This is probably the most important
		   * part of this program! Want to rent this space for commercials?
		   * No, just kidding! Not for sale! ;-) */
#if RPM_VERSION >= 402
		  rc = rpmRunTransactions( rpmdep, showProgress, (void *)((long) NULL),
					   NULL, &probs, (rpmtransFlags) installFlags, 
					   (rpmprobFilterFlags) probFilter);
#else
		  rc = rpmRunTransactions( rpmdep, showProgress, (void *)((long) NULL),
					   NULL, &probs, installFlags, probFilter);
#endif   
		  
		  /* Delete the progressbar-window */
		  delete progressbar;
	       }
	     else
	       {
		 /* Delete packages */
		 
		 /* Show "Uninstalling packages.."-message in command line */
		 cmdwin->Draw( UNINSTALLLINE );

#if RPM_VERSION >= 402
		  rc = rpmRunTransactions( rpmdep, NULL, NULL,
					  NULL, &probs, (rpmtransFlags) installFlags, 
					   (rpmprobFilterFlags) probFilter);
#else
		  rc = rpmRunTransactions( rpmdep, NULL, NULL,
					  NULL, &probs, installFlags, probFilter);
#endif   
	       }
	     
//	     LockTTY( 0 );
	     if (rc < 0) 
	       {
                  #ifdef DEBUG_TO_TERM
	            TermWrite("Transaction error!\n");
                  #endif
		  numFailed += numBinaryPackages;
	       } 
	     else if (rc) 
	       {
                  #ifdef DEBUG_TO_TERM
	            TermWrite("Transaction error-2!\n");
                  #endif
		  numFailed += rc;
		  for( i = 0; i < probs->numProblems; i++ ) 
		    {
		       if (!probs->probs[i].ignoreProblem) 
			 {
#if RPM_VERSION >= 402
			    char *msg = (char *)rpmProblemString(&probs->probs[i]);
#else
			    char *msg = (char *)rpmProblemString(probs->probs[i]);
#endif   
			    free(msg);
			 }
		    }
	       }
	     
	     if( probs ) rpmProblemSetFree( probs );
	     probs = NULL;
	     
	     
	     /******************************************************************/
	     /* Go through all the packages and operate on the list-structures */
	     /******************************************************************/

	     /* Choose which list to work on */
	     if( operation == UNINSTALL )
		  tmpPtr = topListpointer[ INSTALLED ];
	     else
		  tmpPtr = topListpointer[ AVAILABLE ];
	     
	     /* Jump to element after the dummy */
	     if( tmpPtr->straightlink != NULL ) 
	       tmpPtr = (listelement *)tmpPtr->straightlink;
	     
	     /* Loop through the list */
	     while( tmpPtr != NULL )
	       {
		  /* Is the package selected, i.e. did we operate on it? */
		  if( tmpPtr->is_marked )
		    {
		      /* Reset the size and select counters */
		      if( ! installTemporary || (installTemporary && (op_panel == AVAILABLE)))
			{
			  packagePanel[ op_panel ]->ChangeSelectedCount( -1 );
			  packagePanel[ op_panel ]->ChangeSelectedSize(- tmpPtr->size);
			}
		      
		       /* Yep, deselect it */
		       tmpPtr->is_marked = FALSE;
		       switch( operation )
			 {
			  case INSTALL:
			    if( ! showall )
			      {
				 /* Move the package from the right panel to the left */ 
				 tmpPtr = (listelement *)MoveItem( topListpointer[INSTALLED], 
								  AVAILABLE, tmpPtr );
			      }
			    else
			      {
				 /* Showall means that we don't move the package-info, 
				  * we add a new one to the INSTALLED-list and sets the 
				  * installdate for the copy in the available panel */
				 fd = fdOpen( tmpPtr->filename, O_RDONLY, 0);
				 if( fdFileno(fd) >= 0)
				   {
				      /* Read the package header */
				      result = rpmReadPackageHeader( fd, &h, &isSource, NULL, NULL);
				      fdClose(fd);   
				      if( h )
					{
					   tmpPtr3 = AddItem( topListpointer[ INSTALLED ], h, 
							     INSTALLED, tmpPtr->filename );
					   
					   if( tmpPtr3 )
					     tmpPtr3->installdate = time( NULL );

					   headerFree(h);
					}
				   }
				 
				 /* Set the installdate in the available copy */
				 tmpPtr->installdate = time( NULL );
			      }
			
			    break;
			    
			  case UPGRADE:
			    /* Check if there was an old package that was upgraded,
			     * upgrade without an old version installed = installation */
			    if( (tmpPtr3 = CheckPackageExists( INSTALLED, 
							      tmpPtr->name, 
							      NULL, NULL,
							      CASE_SENSITIVE)))
			      {
				 /* Is this package available in the rpm-directory? */
				 if( tmpPtr3->is_available )
				   MoveItem( topListpointer[AVAILABLE], 
					    INSTALLED, tmpPtr3 );
				 else
				   {
				      RemoveItem( tmpPtr3, currentPanel );
				   }
			      }
			    
			    /* Move the package from the right panel to the left */ 
			    tmpPtr = (listelement *)MoveItem( topListpointer[INSTALLED], 
							     AVAILABLE, tmpPtr );
			    break;
			    
			  case UNINSTALL:
			    if( tmpPtr->is_available && ! showall)
			      {
				 tmpPtr = (listelement *)MoveItem( topListpointer[AVAILABLE], 
								    INSTALLED, tmpPtr );
			      }
			    else
			      {
				      /* If we have showall-mode we should set installtime
				       * in the header in the available-list to null to make it
				       * show up dimmed */
				 if( showall )
				   {
				      tmpPtr3 = CheckPackageExists( AVAILABLE,
								   tmpPtr->name, tmpPtr->version,
								   tmpPtr->release, CASE_SENSITIVE );
				      
				      if( tmpPtr3 )
					tmpPtr3->installdate = (time_t) NULL;
				   }
				 
				 tmpPtr = (listelement *)RemoveItem( tmpPtr, 
								    currentPanel );
			      }
			    
			    break;
			 }
		    }
		  else
		    tmpPtr = (listelement *)tmpPtr->straightlink;
	       }
	     /* Move cursors to top-positions in the panel after the install */
	     packagePanel[ INSTALLED ]->SetCurrentItem( 0 );
	     packagePanel[ AVAILABLE ]->SetCurrentItem( 0 );
	     packagePanel[ INSTALLED ]->SetHighestItem( 0 );
	     packagePanel[ AVAILABLE ]->SetHighestItem( 0 );
	     

	     /* Operation OK! */
	     if( numBinaryPackages ) rpmtransFree(rpmdep);	
	     if( db )
	       {
		  rpmdbClose( db );
                  #ifdef DEBUG_TO_TERM
            	     TermWrite("Install: closing rpm-datbase (OK)\n");
                  #endif
	       }
	     return 0;
	  }
	else
	  {
	     /* Operation stopped! */ 
	     if( numBinaryPackages ) rpmtransFree(rpmdep);
             if( db )
	       {
                  #ifdef DEBUG_TO_TERM
		    TermWrite("Install: closing rpm-datbase (operation stopped)\n");
                  #endif 
		  rpmdbClose( db );
	       }
	     return -1;
	  }
     }
   else
     {
	/* User said no at confirmation */
	if( numBinaryPackages ) rpmtransFree(rpmdep);
	if( db )
	  {
             #ifdef DEBUG_TO_TERM
	       TermWrite("Install: closing rpm-datbase (confirm = no)\n");
             #endif 
	     rpmdbClose( db );
	  }
	return -2;
     }
     
   if( db )
     {
	#ifdef DEBUG_TO_TERM
	  TermWrite("Install: closing rpm-datbase (end)\n");
        #endif
	rpmdbClose( db );
     }
}





short ConfirmForceInstall( listelement *tmpPtr )
{			 
   textwin *forcewin;
    char tmpStr[ 128 ];
  
   
   forcewin = new textwin( "File conflicts", "", "", 1, NOWRAP); 
   
   /* Make some noice to wake up the user.. */
   beep();

   sprintf( tmpStr, "\nInstallation of package: %s-%s-%s\n", 
	   tmpPtr->name, tmpPtr->version, tmpPtr->release );
   
   forcewin->AddStr( tmpStr, NOWRAP );
   forcewin->AddStr( "conflicts with files from other installed packages.\n\n", NOWRAP );

   forcewin->AddStr( "\nCommands:\n", NOWRAP );
   forcewin->AddStr( "  Force: Force operation, even if it overwrites files\n", NOWRAP );
   forcewin->AddStr( "  Abort: Abort installation of this package\n", NOWRAP );
   
   /* Confirm installation! */
   cmdwin->Draw( FORCELINE );
   
   if( cmdwin->HandleForceLine() == TRUE )
     {
	delete forcewin;
	return 1;
     }
   else
     {
	delete forcewin;
	return 0;
     }
}



short HandleDependenyConflicts( short operation, listelement *tmpPtr,  
			        struct rpmDependencyConflict * conflicts,
			        int numConflicts)
{			 
   textwin *depwin;
   char tmpStr[ 128 ];
   int i;
   short result;
   
   depwin = new textwin( "Dependencies conflicts", "", "", 1, NOWRAP); 
   
   /* Make some noice to wake up the user.. */
   beep();
   
   depwin->AddStr( "\n", NOWRAP );

   for (i = 0; i < numConflicts; i++) 
     {
	sprintf( tmpStr, "%s", conflicts[i].needsName );
	
	if( conflicts[i].needsFlags )
	  {
	     if (conflicts[i].needsFlags)
	       sprintf( tmpStr, "%s ", tmpStr);
	     
	     if (conflicts[i].needsFlags & RPMSENSE_LESS)
	       sprintf( tmpStr, "%s<", tmpStr);
	     if (conflicts[i].needsFlags & RPMSENSE_GREATER)
	       sprintf( tmpStr, "%s>", tmpStr);
	     if (conflicts[i].needsFlags & RPMSENSE_EQUAL)
	       sprintf( tmpStr, "%s=", tmpStr);
	     if (conflicts[i].needsFlags & RPMSENSE_SERIAL)
	       sprintf( tmpStr, "%sS", tmpStr);
	     
	     if (conflicts[i].needsFlags)
	       sprintf( tmpStr, "%s %s", tmpStr, conflicts[i].needsVersion ); 
	  }
	
	  sprintf( tmpStr, "%s is needed by %s-%s-%s\n", tmpStr, 
		  conflicts[i].byName, conflicts[i].byVersion, conflicts[i].byRelease);
		
	depwin->AddStr( tmpStr, NOWRAP );
     }

   depwin->AddStr( "\n\nCommands:\n", NOWRAP );
   depwin->AddStr( "  Force: Force operation, even if it breaks dependencies\n", NOWRAP );

/********************************** NOT YET IMPLEMENTED **************************
   if( operation <= UPGRADE )
     depwin->AddStr( "  Solve: Install (if available) the packages to solve the conflicts\n", NOWRAP );
   else 
     depwin->AddStr( "  Solve: Do not uninstall packages that are needed\n", NOWRAP );
*/ 
   
   depwin->AddStr( "  Abort: Abort this operation\n", NOWRAP );
   
   
   /* Confirm installation! */
   cmdwin->Draw( DEPENDENCIESLINE );

   result = cmdwin->HandleDependenciesLine();

   delete depwin;

   return result;
}

