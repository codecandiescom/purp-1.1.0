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

#ifndef MAIN_H
#define MAIN_H

#include "config.h"

extern "C" { 
   #include <rpm/rpmlib.h>
} 

#define REAL_INSTALL 1
#define REAL_UNINSTALL 1

#define PURP_VERSION_STRING "PURP 1.1.0 - © 1998-2001 Anders \"Pugo\" Karlsson"
#define PURP_DESCRIPTION_STRING " PURP (Pugo RPM) Version 1.1.0 - A ncurses-based RPM-handler \n"
#define PURP_COPYRIGHT_STRING " Copyright (C) 1998-2001 -- Anders Karlsson <pugo@pugo.org>\n\n"

#define SIGSEGV_MESSAGE "Oops! Caught a SIGSEGV.. Shutting down.."
#define SIGHUP_MESSAGE  "Caught a SIGHUP.. Shutting down.."
#define SIGTERM_MESSAGE  "Caught a SIGKILL.. Shutting down!"

#define MINSIZE_MESSAGE "Purp needs a window of at least 60x14 to work!"

#define  INSTALLED_PANEL_TITLE   "Installed packages"
#define  AVAILABLE_PANEL_TITLE   "Available packages"

#define  LIBRPMALIAS_FILENAME "/usr/lib/rpmpopt"


void quitPurp( short, char * );
short ScanCommandLine( int, char ** );
void InterpretFlag( char **, int *, short *);
void PrintUsageInfo( void );
void InitRPM( void );
short purp_rpmdbOpen( rpmdb *db );

/* Signal handlers */
void sigSEGVhandler( int );
void sigHUPhandler( int );
void sigTERMhandler( int );
void sigWINCHhandler( int );

void resizeInterface( void );
void CheckResize( void );

#endif // MAIN_H
