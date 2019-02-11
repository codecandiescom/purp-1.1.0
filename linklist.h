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

#ifndef LINKLIST_H
#define LINKLIST_H

#include "config.h"


#define CASE_SENSITIVE    0
#define CASE_INSENSITIVE  1

#define DUMMY    0
#define GROUP    1
#define PACKAGE  2   

extern "C" {
      #include <rpm/rpmlib.h>
//      #include <rpm/header.h>
//      #include <rpm/dbindex.h>
}


struct listelement
{
   char name[ 64 ];
   char version[ 32 ];
   char release[ 32 ];
   time_t installdate;
   time_t builddate;
   char group[ 64 ];
   long size;           /* The package-size, or if it's a folder its number of packages */

   char *summary;
   char *description;
   char *filename;

   int  elementtype;    /* Type of element, dummy, group or package */
   int  selected_item;  /* The item selected when moving down in hierarchy */
   int  top_item;       /* The highest selected item on a list */
   bool is_marked;      /* If the element is marked */
   bool is_available;   /* If the element is an available package */
   
   listelement  *link;               /* Next element */
   listelement  *prevlink;           /* Previous element */
   
   listelement  *straightlink;       /* Next element for straight list */
   listelement  *straightprevlink;   /* Previous element for straight list */

   listelement  *grouplink;          /* Link to the group-list */
};


listelement *AddItem( listelement *, Header, short, char * );
listelement *GetGroup( listelement * listPtr, char * group );
listelement *GetCreateGroup( listelement * listPtr, char * group );
listelement *AddGroup ( listelement *, char *, char *);
listelement *MoveItem( listelement *, int, listelement * );
listelement *FindGroup( listelement *, char *);
listelement *ReturnItemNo( listelement *, int );
listelement *CreateDummy( listelement *);
listelement *CreateListItem( listelement * lListpointer );
listelement *RemoveItem( listelement *, int );
void EraseGroupHierarchy( listelement * itemPtr, int );
void UnlinkItem( int panel, listelement * itemPtr );
void LinkItem( listelement * groupPtr, listelement * itemPtr,
	       listelement * destinationList, listelement * before_item );
void PrintList( listelement * lListpointer );
void EraseList(listelement *listpointer);
void EraseRecursive( listelement * itemPtr );
void FreeItem( listelement * itemPtr );

bool ToggleMark( listelement *listpointer, int item );
listelement * CheckPackageExists( int, char *, char *, char *, short );
void UpdatePackagesCounter( char *, short, short );


#endif // LINKLIST_H
