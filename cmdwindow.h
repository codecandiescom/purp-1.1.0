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

#ifndef CMDWINDOW_H
#define CMDWINDOW_H

#include "window.h"
#include "linklist.h"
#include "config.h"

#define MAINLINE            0
#define LISTLINE            1
#define CONFIGLINE          2
#define CONFIRMLINE         3
#define FORCELINE           4
#define ABOUTLINE           5
#define DEPENDENCIESLINE    6
#define SEARCHLINE          7
#define UNINSTALLLINE       8

#define MAIN_CMD_LINE "^Install ^Upgrade ^Erase ^Follow ^Description ^List ^Search ^About ^Quit"
#define LIMITED_MAIN_CMD_LINE "^Description ^List ^Search ^About ^Quit"
#define LIST_CMD_LINE "_List:_ ^Files ^Scripts ^Dependencies ^Abort"
#define CONFIRM_CMD_LINE "_Confirm:_ ^Yes ^No"
#define FORCE_CMD_LINE "_Confirm:_ ^Force ^Abort"
#define ABOUT_CMD_LINE "_[ Purp 1.1.0 - (C)1998-2001 Anders Karlsson - press \"A\" for licence info ]"
#define DEPENDENCIES_CMD_LINE "_Conflicts:_ ^Force ^Abort"
#define SEARCH_CMD_LINE "_Package-name (or part of): "
#define UNINSTALL_CMD_LINE "_Uninstalling packages..."

class cmdwindow : public window
{
 public:
   cmdwindow( int, int );

   void Draw( short );
   void Draw( void );

   void HandleListLine( listelement *lListpointer, int panel, int option );
   bool HandleConfirmLine( void );
   bool HandleForceLine( void );
   short HandleDependenciesLine( void );
   void HandleSearchLine( int panel, char * name );

 private:
   void DrawCmdLine( char * );
   
   short currentBar;

};



#endif  /* CMDWINDOW_H */

