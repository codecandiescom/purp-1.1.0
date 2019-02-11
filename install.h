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

#ifndef INSTALL_H
#define INSTALL_H

#include "config.h"


#define INSTALL 1
#define UPGRADE 2
#define UNINSTALL 3

#define INSTALL_TEMPORARY 1
#define INSTALL_MARKED 0

#define DEP_FORCE 1
#define DEP_SOLVE 2
#define DEP_ABORT 3

#define INSTALL_PERCENT         (1 << 0)
#define INSTALL_HASH            (1 << 1)
#define INSTALL_NODEPS          (1 << 2)
#define INSTALL_NOORDER         (1 << 3)
#define INSTALL_LABEL           (1 << 4)  /* set if we're being verbose */
#define INSTALL_UPGRADE         (1 << 5)



#define UNINSTALL_NODEPS        (1 << 0)
#define UNINSTALL_ALLMATCHES    (1 << 1)


short InstallPackages( short, short );
short ConfirmForceInstall( listelement * );
short HandleDependenyConflicts( short, listelement *, struct rpmDependencyConflict *, int);

static void * showProgress(const Header h, const rpmCallbackType what,
			                              const unsigned long amount,
			                              const unsigned long total,
			                              const void * pkgKey, void * data);


#endif  // INSTALL_H
