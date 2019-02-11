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
#include <curses.h>
#include <ctype.h>
#include <time.h>

extern "C" {
      #include <rpm/rpmlib.h>
}

#include "linklist.h"
#include "tools.h"
#include "main.h"
#include "panel.h"

extern int currentPanel;
extern panel *packagePanel[];
extern char      *availdir;

extern listelement *listpointer[2];
extern listelement *topListpointer[2];

extern bool showall;

/***********************************************************************
 * AddItem: Adds a new element to the linked list
 ***********************************************************************/

/***********************************************************************
 * The packages are internally linked in two chains, one straight, 
 * which is very useful for traversing all packages, and one 
 * hierararchial, which is used for the package-hierarchy-views. 
 * 
 * Link-pointers:
 * --------------
 * 
 * The straight chain:     straightlink       (next item)
 *                         straightprevlink   (previous item)
 * 
 * 
 * The hierarchial chain:  link       (next item)
 *                         prevlink   (previos item)
 *                         grouplink  (link down into group)
 * 
 **********************************************************************/



listelement * AddItem( listelement * lListpointer, Header h, short panel, char * filename )
{
   char * name;
   char * version;
   char * release;
   char * group;
   time_t  * installdate;
   time_t  * builddate;
   long * size;
   char * summary;
   
   char * distribution;
   char * vendor;
   char * buildhost;
   char * sourcerpm;
   
   char * description; 
   char tmpstr[256] = ""; 
   char *groupname;
   
   int_32 count, type;
   listelement *listtail;
   listelement *listPtr = lListpointer, *groupPtr;
   listelement *tmpPtr, *isAvailPtr = NULL, *tmpBackPtr;

   
   /* Get package-information */
   headerGetEntry(h, RPMTAG_NAME, &type, (void **) &name, &count); 
   headerGetEntry(h, RPMTAG_VERSION, &type, (void **) &version, &count);
   headerGetEntry(h, RPMTAG_RELEASE, &type, (void **) &release, &count);


   if( (panel == INSTALLED) ||
       ( (panel == AVAILABLE) && 
	((! (isAvailPtr = CheckPackageExists( INSTALLED, name, version, release, CASE_SENSITIVE ))) || showall)))
     {
	headerGetEntry(h, RPMTAG_GROUP, &type, (void **) &group, &count);

	/* Get pointer to the group this package should reside in */
	listPtr = groupPtr = GetCreateGroup( listPtr, group );
	
	/* Go to the end of the list */
	if( listPtr->prevlink != NULL )
	  listPtr = (listelement *)listPtr->prevlink;

	
	
	/* Put package at right place, sorted */
	while( 1 )
	  {
	     if( listPtr->elementtype == GROUP ) break;
	     
	     if( strcmp( name, listPtr->name ) <= 0 )
	       {
		  listPtr = (listelement *)listPtr->prevlink;
	       }
	     else
	       break;
	  }

	/* Get tail */
	listtail = (listelement *)listPtr->link;
	

	/* Create the new entry and fill it with nice info */
	listPtr->link = CreateListItem( listPtr->link );
	tmpPtr = listPtr;
	listPtr = (listelement *)listPtr->link;  /* enter new element */
	listPtr->link = listtail;
	listPtr->prevlink = (struct listelement *)tmpPtr;
	
	/* Store pointer to last entry in the dummy */
	if( listPtr->link == NULL )
	  groupPtr->prevlink = (struct listelement *)listPtr;
	else
	  listPtr->link->prevlink = (struct listelement *)listPtr;

	/* Store pointer to group this package resides in into the package */
	listPtr->grouplink = groupPtr;
	
	/* Get package-information */
	headerGetEntry(h, RPMTAG_SIZE, &type, (void **) &size, &count);
	
	headerGetEntry(h, RPMTAG_BUILDTIME, &type, (void **) &builddate, &count);
	headerGetEntry(h, RPMTAG_INSTALLTIME, &type, (void **) &installdate, &count);
	
	headerGetEntry(h, RPMTAG_SUMMARY, &type, (void **) &summary, &count);
	headerGetEntry(h, RPMTAG_DESCRIPTION, &type, (void **) &description, &count);
	
	if( name ) strcpy( listPtr->name,  name );
	if( version ) strcpy( listPtr->version, version );
	if( release ) strcpy( listPtr->release, release );
	
	if( group ) strcpy( listPtr->group, group );
	
	if( installdate )
	  {
	     listPtr->installdate = *installdate;
	  }
	else if( isAvailPtr )
	  {
	     listPtr->installdate = isAvailPtr->installdate;
	  }
	else
	  {
	     listPtr->installdate = (time_t) 0;
	  }
	
	if( builddate )
	  listPtr->builddate = *builddate;
	
	if( size )
	  listPtr->size = *size;
	
		
	/* Alloc memory for the package-summary */
	if( summary )
	  {
	     listPtr->summary = (char *) xmalloc( strlen( summary ) + 1 );
	     strcpy( listPtr->summary, summary );
	  }
	else
	  {
	     listPtr->summary = (char *) xmalloc( 2 );
	     strcpy( listPtr->summary, "" );
	  }
	
	
	/* Alloc memory for the package-description */
	if( description ) 
	  {
	     listPtr->description = (char *) xmalloc( strlen( description ) + 1 );
	     strcpy( listPtr->description, description );
	  }
	else
	  {
	     listPtr->description = (char *) xmalloc( 2 );
	     strcpy( listPtr->description, "" );
	  }
	
	
	/* Alloc memory for the package-filename */
	if( filename ) 
	  {
	     listPtr->filename = (char *) xmalloc( strlen(filename) + strlen(availdir) + 5 );
	     sprintf( listPtr->filename, "%s%s", availdir, filename );
	  }
	else
	  {
	     listPtr->filename = (char *) xmalloc( 2 );
	     strcpy( listPtr->filename, "" );
	  }

	
	UpdatePackagesCounter( group, +1, panel );
		
	listPtr->selected_item = 0;
	listPtr->is_marked = FALSE;
	listPtr->elementtype = PACKAGE;
	
	if( panel == AVAILABLE )
	  listPtr->is_available = TRUE;
	else
	  listPtr->is_available = FALSE;
	
	/* Link the element up in the straight chain */
	tmpPtr = (listelement *)lListpointer;
	
	/* Go to the end of the list */
	if( tmpPtr->straightprevlink != NULL )
	  tmpPtr = (listelement *)tmpPtr->straightprevlink;
	
	tmpPtr->straightlink = (struct listelement *)listPtr;
	listPtr->straightlink = NULL;
	listPtr->straightprevlink = (struct listelement *)tmpPtr;
	lListpointer->straightprevlink = (struct listelement *)listPtr;
	return listPtr;
     }
   else
     {
	isAvailPtr->is_available = TRUE;

	/* Alloc memory for the package-filename */
	isAvailPtr->filename = (char *) realloc( isAvailPtr->filename, 
						 strlen( filename ) + strlen( availdir ) + 5 );

	sprintf( isAvailPtr->filename, "%s%s", availdir, filename );
	return listPtr;

     }
   
   return lListpointer;
}


/***********************************************************************
 * Return pointer to group. Create the group if it doesn't exist
 ***********************************************************************/
listelement * GetCreateGroup( listelement * listPtr, char * group )
{
   char tmpstr[256] = ""; 
   char grouppath[1024] = "";
   char *groupname;
   listelement * tmpPtr, * groupPtr, * tmpGroupPtr = listPtr;

   strcpy( tmpstr, group );
   groupname = strtok( tmpstr, "/" );
	
   // Create hierarchy
   while( groupname != NULL )
     {
	strcat( grouppath, "/" );
	strcat( grouppath, groupname );
	
	tmpPtr = tmpGroupPtr;
	
	/* Create the group if it doesn't exist */
	if( (groupPtr = FindGroup( tmpPtr, groupname )) == NULL )
	  {
		  tmpPtr = tmpGroupPtr;
		  groupPtr = AddGroup( tmpPtr, grouppath, groupname );
	  }
	
	/* Enter the group */
	tmpGroupPtr = (listelement *)groupPtr->grouplink;
	
	groupname = strtok( NULL, "/" );
     }
   return tmpGroupPtr;
}

/***********************************************************************
 * Return pointer to group. 
 ***********************************************************************/
listelement * GetGroup( listelement * listPtr, char * group )
{
   char tmpstr[256] = ""; 
   char *groupname;
   listelement * tmpPtr, * groupPtr, * tmpGroupPtr = listPtr;

   strcpy( tmpstr, group );
   groupname = strtok( tmpstr, "/" );
	
   // Go down in hierarchy
   while( groupname != NULL )
     {
	tmpPtr = tmpGroupPtr;
	
	/* Create the group if it doesn't exist */
	if( (groupPtr = FindGroup( tmpPtr, groupname ) ))
	  {
	     /* Enter the group */
	     tmpGroupPtr = (listelement *)groupPtr->grouplink;
	  }
	else
	  return NULL;
	
	groupname = strtok( NULL, "/" );
     }
   return tmpGroupPtr;
}



/***********************************************************************
 * AddGroup: Adds a new group to the linked list
 ***********************************************************************/
listelement * AddGroup (listelement * lListpointer, char * group, char * name )
{
   listelement *listtail = (listelement *)lListpointer->link;
   listelement *tmpListPtr = lListpointer;
   listelement *listPtr;
   listelement *tmpList;

   /* Check if there is a ".." at the beginning, and if there is,
    * jump to the position after, so it remains at the beginning.. */

   if( tmpListPtr->link != NULL )
     if( ! strcmp( "..", tmpListPtr->link->name ))
     {
	if( listtail != NULL )
	  listtail = (listelement *)listtail->link;
	tmpListPtr = (listelement *)tmpListPtr->link;
     }   


   /* Put group at right place, sorted */
   while( 1 )
     {
	if( tmpListPtr->link == NULL ) break;
	
	if( tmpListPtr->link->elementtype != GROUP ) break;
	
	
	if( strcmp( name, tmpListPtr->link->name ) <= 0 )
	  break;
	else
	  {
	     listtail = (listelement *)listtail->link;
	     tmpListPtr = (listelement *)tmpListPtr->link;
	  }
     }
  
      

   /* Allocate memory for the group-object and enter that object */
   tmpListPtr->link = CreateListItem( tmpListPtr->link );
   tmpList = (listelement *)tmpListPtr;
   tmpListPtr = (listelement *)tmpListPtr->link;

   
   /* Link the rest of the chain after the new object */
   tmpListPtr->link = (struct listelement *)listtail;
   
   /* Link back */
   tmpListPtr->prevlink = (listelement *)tmpList;


   /* Check if the new group is at the end of the chain, 
    * if it is we have to update the 'prevlink' in the dummy.
    * If not at end we have to update 'prevlink' of the 
    * following object. */
   
   if( tmpListPtr->link == NULL )
     lListpointer->prevlink = tmpListPtr;
   else
     tmpListPtr->link->prevlink = tmpListPtr;
   
   /* Create a dummy-element for the new group */
   tmpListPtr->grouplink = (struct listelement *)CreateDummy( (listelement *)tmpListPtr->grouplink );
   
   tmpListPtr->description = (char *) xmalloc( 1 );

   /* Set some values for the group */
   tmpListPtr->size = 0;   /* New groups of course have no packages */
   tmpListPtr->elementtype = GROUP;
   tmpListPtr->is_marked = FALSE;
   strcpy( tmpListPtr->name,  name );
   strcpy( tmpListPtr->group,  group );
   

   /* Create the .. (previous dir) group */
   listPtr = (listelement *)tmpListPtr->grouplink; /* Enter group */
   listPtr->size = 0;    /* New groups of course have no packages */
   strcpy( listPtr->group,  group );
   listPtr->link = CreateListItem( listPtr->link );
   tmpList = listPtr;
   listPtr = (listelement *)listPtr->link;
   listPtr->link = NULL;
   listPtr->prevlink = (struct listelement *)tmpList; /* Link back to previous entry */
   listPtr->description = (char *) xmalloc( 1 );
   strcpy( listPtr->name, ".." );
   strcpy( listPtr->group,  group );
   listPtr->selected_item = 0;
   listPtr->top_item = 0;
   listPtr->size = 0;
   listPtr->elementtype = GROUP;
   listPtr->is_marked = FALSE;
   listPtr->grouplink = (struct listelement *)lListpointer;

   tmpList->prevlink = (struct listelement *)listPtr;

   return tmpListPtr;
}


/***********************************************************************
 * UnlinkItem: Unlink an item from a list
 ***********************************************************************/
void UnlinkItem( int panel, listelement * itemPtr )
{
   listelement *tmpPtr, *groupPtr2;
   listelement * next;
   listelement * prev;
   listelement * snext;
   listelement * sprev;
   
#ifdef DEBUG_TO_TERM
     TermWrite( "Unlinking listelement: %s\n", itemPtr->name );
#endif
    
   /********************************************/
   /* Remove the package from the linked lists */
   /********************************************/
   
   tmpPtr = itemPtr;
   next = (listelement *)tmpPtr->link;
   prev = (listelement *)tmpPtr->prevlink;
   if( tmpPtr->elementtype != GROUP )
     {
	snext = (listelement *)tmpPtr->straightlink;
	sprev = (listelement *)tmpPtr->straightprevlink;
     }
   
  /* If the deleted package is last in chain then link the dummy->prev 
   * to deleted->prev. Otherwise just link next->prev to deleted->prev. */
   
   if( next == NULL )
     {
	itemPtr->grouplink->prevlink = (struct listelement *)prev;
     }
   else
     next->prevlink = (struct listelement *)prev;
   
   /* link prev->link to item after the deleted */
   prev->link = (struct listelement *)next;

   if( tmpPtr->elementtype != GROUP )
     {
	/* Handle the straight-linking, link next->sprev to sprev */
	if( snext == NULL )
	  topListpointer[ panel ]->straightprevlink = 
	                            (struct listelement *)sprev;
	else
	  snext->straightprevlink = (struct listelement *)sprev;

	/* Link sprev->snext to snext */
	sprev->straightlink = (struct listelement *)snext;
     }
}


/***********************************************************************
 * LinkItem: Link an item into a list after 'before_item'.
 ***********************************************************************/
void LinkItem( listelement * groupPtr, listelement * itemPtr,
	       listelement * destinationList, listelement * before_item )
{
   listelement * tmpPtr, * listPtr;
   listelement * listtail;
   
#ifdef DEBUG_TO_TERM
     TermWrite( "Linking listelement: '%s' after item '%s'\n", 
	               itemPtr->name, before_item->name );
#endif

   /* Get tail */
   listtail = (listelement *)before_item->link;
	
   /* Insert the item */
   before_item->link = itemPtr;
   itemPtr->link = listtail;
   itemPtr->prevlink = (struct listelement *)before_item;

   /* Store pointer to last entry in the dummy */
   if( itemPtr->link == NULL )
     groupPtr->prevlink = (struct listelement *)itemPtr;
   else
     itemPtr->link->prevlink = (struct listelement *)itemPtr;

   /* Store pointer to group this package resides in into the package */
   itemPtr->grouplink = groupPtr;
   
   /* Link the element up in the straight chain */
   listPtr = (listelement *)destinationList;

   /* Go to the end of the list */
   if( listPtr->straightprevlink != NULL )
     listPtr = (listelement *)listPtr->straightprevlink;

   listPtr->straightlink = itemPtr;
   itemPtr->straightlink = NULL;
   itemPtr->straightprevlink = (struct listelement *)listPtr;
   destinationList->straightprevlink = (struct listelement *)itemPtr;
}

/***********************************************************************
 * MoveItem: Move an element from current list to another list
 ***********************************************************************/

listelement * MoveItem( listelement * destinationList, 
		       int sourcePanel, 
		       listelement * sourceElement )
{
   listelement *listPtr = destinationList, *groupPtr;
   listelement *next_straight_item;
   
#ifdef DEBUG_TO_TERM
     TermWrite( "Moving listelement: %s-%s-%s\n", 
	     sourceElement->name, 
	     sourceElement->version, 
	     sourceElement->release );
#endif

   /******************************************** 
    * Remove the package from the source list  * 
    ********************************************/   

   next_straight_item = (listelement *)sourceElement->straightlink;

   /* Remove from source-panel */
   UnlinkItem( sourcePanel, sourceElement );
   UpdatePackagesCounter( sourceElement->group, -1, sourcePanel );

   
   /******************************************** 
    * Move the package to the destination list * 
    ********************************************/   
     
   /* Get pointer to the group this package should reside in */
   groupPtr = listPtr = GetCreateGroup( destinationList, sourceElement->group );
      
   /* Go to the end of the list */
   if( listPtr->prevlink != NULL )
     listPtr = (listelement *)listPtr->prevlink;
       
   /* Search upwards for correct place to put this package */
   while( 1 )
     {
	if( listPtr->elementtype == GROUP ) break;
	
	if( strcmp( sourceElement->name, listPtr->name ) <= 0 )
	  listPtr = (listelement *)listPtr->prevlink;
	else
	  break;
     }

   /* Link the item in the destination-list */
   LinkItem( groupPtr, sourceElement, destinationList, listPtr );
   
   /* Set package-flags */
   sourceElement->is_marked = FALSE;

   /* If we installed the package then set installdate to current
    * time and if we erased then set installdate to NULL.
    * The most exact way would be to open the rpmdb and read the 
    * correct installdate for the specific package there, but
    * that would take lot of time and the correctness isn't 
    * so important that some seconds difference causes any troubles. */
   
   if( sourcePanel == INSTALLED )
     sourceElement->installdate = (time_t) NULL;  // Erase
   else
     sourceElement->installdate = time( NULL );


   UpdatePackagesCounter( sourceElement->group, +1, 
			( sourcePanel == INSTALLED ) ? AVAILABLE : INSTALLED);

   /* Return pointer to next item in list */
   return next_straight_item;

} /* MoveItem */



/**************************************************************************
 * RemoveItem
 **************************************************************************/

listelement * RemoveItem( listelement * itemPtr, int lPanel )
{
   listelement * tmpPtr = itemPtr;
   listelement * next_straight_item;

#ifdef DEBUG_TO_TERM
   TermWrite( "RemoveItem: %s\n", itemPtr->name );
#endif
 
   next_straight_item = (listelement *)tmpPtr->straightlink;

   /* Remove from source-panel */
   UnlinkItem( lPanel, tmpPtr );

   /* Update counter that keeps track of packages in tree */
   UpdatePackagesCounter( tmpPtr->group, -1, lPanel );
   
   FreeItem( tmpPtr );
   return( next_straight_item );
}



/**************************************************************************
 * EraseGroupHierarchy
 **************************************************************************/

void EraseGroupHierarchy( listelement * itemPtr, int panel )
{
   listelement * tmpPtr;
   listelement * tmpPtr2;
   
#ifdef DEBUG_TO_TERM
   TermWrite( "EraseGroupHierarchy: %s\n", itemPtr->name );
#endif
   
   tmpPtr = (listelement *)itemPtr->grouplink;
   
   /* Remove from source-panel */
   UnlinkItem( panel, itemPtr );
   FreeItem( itemPtr );
   
   /* Erase all folders and files in this folder */
   while( tmpPtr != NULL )
     {
	/* If we have a folder, recurse down and erase it */
	if( (tmpPtr->elementtype == GROUP) && ( strcmp( "..", tmpPtr->name ) ))
	  {
	     tmpPtr2 = tmpPtr;
	     EraseRecursive( tmpPtr2->grouplink );
	     tmpPtr = (listelement *)tmpPtr -> link;
	     FreeItem( tmpPtr2 );
	  }
	else
	  {	     
	     /* Else just remove the item */
	     tmpPtr2 = tmpPtr;
	     tmpPtr = (listelement *)tmpPtr -> link;
	     FreeItem( tmpPtr2 );
	  }
     } 
}


/**************************************************************************
 * FreeItem
 * Erase item and it's objects
 **************************************************************************/
void FreeItem( listelement * itemPtr )
{
   if( itemPtr->summary )      free( itemPtr->summary );
   if( itemPtr->description )  free( itemPtr->description );
   if( itemPtr->filename)      free( itemPtr->filename );
   free( itemPtr );
}


/**************************************************************************
 * EraseRecursive
 * (Only used from EraseGroupHierarchy!)
 **************************************************************************/
void EraseRecursive( listelement * itemPtr )
{
   listelement * tmpPtr = itemPtr;
   listelement * tmpPtr2;
   
#ifdef DEBUG_TO_TERM
   TermWrite( "EraseRecursive: %s", itemPtr->name );
#endif

   /* Erase all folders and files in this folder */
   while( tmpPtr != NULL )
     {
	/* If we have a folder, recurse down and erase it */
	if( tmpPtr->elementtype == GROUP && ( strcmp( "..", tmpPtr->name ) ))
	  {
	     tmpPtr2 = tmpPtr;
	     EraseRecursive( tmpPtr2->grouplink );
	     tmpPtr = (listelement *)tmpPtr -> link;
	     FreeItem( tmpPtr2 );
	  }
	else
	  {	     
	     tmpPtr2 = tmpPtr;
	     tmpPtr = (listelement *)tmpPtr -> link;
	     FreeItem( tmpPtr2 );
	  }
     } 
}



/***********************************************************************
 * FindGroup: Searches the list for a named group, and returns it
 ***********************************************************************/
listelement * FindGroup( listelement * lListpointer, char * name )
{
   listelement *tmpPtr = lListpointer;

   char tmpName1[128];
   char tmpName2[128];
   
   strcpy( tmpName1, name );
   tmpName1[0] = tolower( tmpName1[0] );
   
   while( tmpPtr != NULL )
     {
	strcpy( tmpName2, tmpPtr->name );
	tmpName2[0] = tolower( tmpName2[0] );
	if( strcmp( tmpName1, tmpName2 ) == 0 )
	  if( tmpPtr->elementtype == GROUP )
	    return tmpPtr;
	
	tmpPtr = (listelement *)tmpPtr -> link;
     } 

   return NULL;
}

/***********************************************************************
 * CreateDummy():
 * Creates the first "dummy element" in the linked list.
 *
 * Arguments: pointer to the linked list
 * Returns:   pointer to the linked list
 ***********************************************************************/

listelement * CreateDummy( listelement * lListpointer )
{
   lListpointer = (struct listelement *)xmalloc (sizeof( listelement ));
   lListpointer->link = NULL;
   lListpointer->prevlink = NULL;
   lListpointer->straightlink = NULL;
   lListpointer->straightprevlink = NULL;
   lListpointer->elementtype = DUMMY; /* mark dummy as dummy */
   lListpointer->top_item = 0;
   lListpointer->selected_item = 0;
   lListpointer->summary = NULL;
   lListpointer->description = NULL;
   lListpointer->filename = NULL;
     
   strcpy( lListpointer->name, "Dummy" );
   strcpy( lListpointer->version, "Dummy" );
   strcpy( lListpointer->release, "Dummy" );
   
   return lListpointer;
}

listelement * CreateListItem( listelement * lListpointer )
{
   lListpointer = (struct listelement *)xmalloc (sizeof( listelement ));
   lListpointer->link = NULL;
   lListpointer->prevlink = NULL;
   lListpointer->straightlink = NULL;
   lListpointer->straightprevlink = NULL;
   lListpointer->grouplink = NULL;
   lListpointer->elementtype = PACKAGE;
   lListpointer->top_item = 0;
   lListpointer->selected_item = 0;
   lListpointer->summary = NULL;
   lListpointer->description = NULL;
   lListpointer->filename = NULL;
   
   return lListpointer;
}

/*********************************************************************
 * ReturnItemNo( listelement *lListpointer, int listnumber, short panel )
 * 
 * Returns: the listelement with number 'listnumber'
 * 
 * Traverses the list 'lListpointer' and returns the item that is
 * number 'listnumber'..
 **********************************************************************/

listelement * ReturnItemNo( listelement *lListpointer, 
			    int listnumber )
{
   listelement *localList = lListpointer;   
   int count = -1;  /* Start at -1 to jump over the dummy */
   
   while( (count < listnumber) && ( localList->link != NULL ) )
     {
	localList = (listelement *)localList->link;
	count++;
     }
   return localList; 
}



/*********************************************************************
 * bool ToggleMark( listelement *lListpointer, int item, short panel ) 
 * 
 * Returns: TRUE if the element 'item' of list 'lListpointer' 
 *          was a package, and therefore toggled.
 * 
 * Toggles the is_marked-flag for the element, so it will show 
 * up marked in the panels.
 **********************************************************************/

bool ToggleMark( listelement *lListpointer, int item ) 
{
   listelement *element_to_toggle = ReturnItemNo( lListpointer, item );
   
   if(( element_to_toggle->elementtype != GROUP ) &&
       ((element_to_toggle->installdate == (time_t) 0) || (currentPanel == INSTALLED)))
     {
	if( element_to_toggle->is_marked )
	  {
	     element_to_toggle->is_marked = FALSE;
	     packagePanel[ currentPanel ]->ChangeSelectedCount( -1 );
	     packagePanel[ currentPanel ]->ChangeSelectedSize( -element_to_toggle->size );
	  }
	else
	  {
	     element_to_toggle->is_marked = TRUE;
	     packagePanel[ currentPanel ]->ChangeSelectedCount( 1 );
	     packagePanel[ currentPanel ]->ChangeSelectedSize( element_to_toggle->size );
	  }
	return TRUE;
     }
   else
     {
	return FALSE;
     }
}


/*********************************************************************
 * void EraseList( listelement * lListpointer )
 * 
 * Removes all entrys from the linked list 'lListpointer'
 **********************************************************************/

void EraseList( listelement * lListpointer )
{
   listelement * next;
   
   while( lListpointer != NULL )
     {
	next = (listelement *)lListpointer->straightlink;
	FreeItem( (listelement *)lListpointer );
	lListpointer = (listelement *)next;
     }
}


/***********************************************************************
 * CheckPackageExists
 * 
 * Returns: The listelement that mathches name-version-release
 *          If it doesn't exist then return NULL
 * 
 * Searches through all packages for one of the panels and checks
 * if the package with name name-version-release exists.
 ***********************************************************************/

listelement * CheckPackageExists( int panel, char * name, char * version, 
				  char * release, short case_mode )
{

   listelement *tmpPtr = topListpointer[ panel ];
   char * starpos;
   size_t cmplen = strlen( name );
   
   char list_name[128];
   char list_version[64];
   char list_release[64];

   char arg_name[128];
   char arg_version[64];
   char arg_release[64];
   
   strcpy( arg_name, name );
   if( version ) strcpy( arg_version, version );
   if( release ) strcpy( arg_release, release );

   #ifdef DEBUG_TO_TERM
     TermWrite( "CheckPackageExists: looking for file '%s-%s-%s', %s\n",
	       name, version, release, (case_mode == CASE_INSENSITIVE) ? "(Case insensitive)" : "(Case sensitive)" );
   #endif
   
   
   /* We don't want to search the dummy.. */   
   if( tmpPtr->straightlink != NULL ) 
     tmpPtr = (listelement *)tmpPtr->straightlink;
 
   if( case_mode == CASE_INSENSITIVE )
     {
	while( tmpPtr != NULL )
	  {
	     strcpy( list_name, tmpPtr->name );
	     if( strncmp( StrToLower(list_name), StrToLower(arg_name), cmplen ) == 0 )
	       {
		  strcpy( list_version, tmpPtr->version );
		  if( (version == NULL) || (strcmp( StrToLower(list_version), StrToLower(arg_version) ) == 0 ))
		    {
		       strcpy( list_release, tmpPtr->release );
		       if( (release == NULL) || (strcmp( StrToLower(list_release), StrToLower(arg_release) ) == 0 ))
			 {  
			    return tmpPtr;
			 }
		    }
	       }
		       
	     tmpPtr = (listelement *)tmpPtr->straightlink;
	  }
     }
   else
     {
	while( tmpPtr != NULL )
	  {
	     if( strncmp( tmpPtr->name, name, cmplen ) == 0 )
	       {
		  if( (version == NULL) || (strcmp( tmpPtr->version, version ) == 0 ))
		    {
		       if( (release == NULL) || (strcmp( tmpPtr->release, release ) == 0 ))
			 {  
			    return tmpPtr;
			 }
		    }
	       }
	     tmpPtr = (listelement *)tmpPtr->straightlink;
	  }
     }
   
   return NULL;
}

/**********************************************************************
 * UpdatePackagesCounter
 * 
 * Change the values of the packages-counters for folders.
 * Go down in the hierarchy, through all folders in the 
 * package-path, and change their values.
 **********************************************************************/
void UpdatePackagesCounter( char * group, short change, short lPanel )
{
   char tmpstr[512] = ""; 
   char *groupname;

   listelement *groupPtr = topListpointer[ lPanel ];
   listelement *prevGroupPtr;
   listelement *cursorPtr;

   strcpy( tmpstr, group );
   groupname = strtok( tmpstr, "/" );
   
   while( groupname != NULL )
     {
	cursorPtr = groupPtr;
	/* Find the group with name == groupname */
	groupPtr = FindGroup( groupPtr, groupname );

	/* Update the package-counter */
	groupPtr->size += change;
	groupPtr->grouplink->size += change;

	if( groupPtr->size == 0)
	  {
	     listpointer[lPanel] = cursorPtr;
//	     table[lPanel].highest_item_on_list = 0;
//	     table[lPanel].new_option = 0; 
	     EraseGroupHierarchy( groupPtr, lPanel );
	     groupname = NULL;
	     break;
	  }
	else
	  {
	     /* Enter the group */
	     groupPtr = (listelement *)groupPtr->grouplink;
	     
	     groupname = strtok( NULL, "/" );
	  }
     }
   
   /* Check if the available-panel is empty, if so: change panel */
   if( lPanel == AVAILABLE )
     {
	groupPtr = topListpointer[ lPanel ];
	if( groupPtr->straightlink == NULL )
	  {
	     currentPanel = INSTALLED;
	  }
     }
}
