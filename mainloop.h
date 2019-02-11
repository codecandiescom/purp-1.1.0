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

#ifndef MAINLOOP_H
#define MAINLOOP_H

#include "config.h"

// The curses-headers doesn't seem to have a constant for the
// tab-key.. Strange.. Well, let's make one! 

#define KEY_TAB 0x09

void MainLoop( listelement *lListpointer[2] );

void HandleQuit( void );
void HandleEnter( listelement *listpointer[2] );
void HandleKeyUp( listelement *listpointer[2] );
void HandleKeyDown( listelement *listpointer[2] );
void HandleKeyPageUp( listelement *listpointer[2] );
void HandleKeyPageDown( listelement *listpointer[2] );
void HandleKeyInsert( listelement *listpointer[2] );
void HandleDescribePackage( listelement *listpointer[2] );
void HandleListPackage( listelement *listpointer[2] );
void HandleKeySearch( listelement *listpointer[2] );
void HandleKeySwitchPanel( listelement *listpointer[2] );
void HandleKeyInstall( listelement *listpointer[2], short operation );
void HandleKeyErase( listelement *listpointer[2] );
void HandleAbout( listelement *listpointer[2] );
void HandleRedraw( listelement *listpointer[2] );
void HandleKeyFollow( listelement *listpointer[2] );

#endif // MAINLOOP

