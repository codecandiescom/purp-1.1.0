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

#ifndef CONFCLASS_H
#define CONFCLASS_H

class ConfList;

class Conf
{
 public:
   Conf( void );
   ~Conf( void );
   
   bool LoadFile( const char * filename );
   bool Add( char * key, char * value );
   char * Search( char * key );
   char * Search( char * key, char * otherwise );
   int    Search( char * key, int otherwise );

 private:
   bool FreeData();
   bool config_exists;
   ConfList * cl;   
};


/* =============================================================== */
/*  ConfItem - Class for items in a conf-list                      */
/* =============================================================== */

class ConfItem
{
 public:
   ConfItem( char * key, char * value );
   ~ConfItem( void );

   bool AddAfter( ConfItem * item );

   char * GetKey( void );
   char * GetValue( void );
   char * Match( char * key );

   ConfItem * GetNext( void );
   ConfItem * GetPrev( void );
   void SetPrev( ConfItem * ); 
   void SetNext( ConfItem * ); 
		
 protected:
   ConfItem * next;
   ConfItem * prev;
   char * key;
   char * value;
};


/* =============================================================== */
/*  ConfList - A linked-list-class for config-values               */
/* =============================================================== */

class ConfList
{
 public:
   ConfList( void );
   ~ConfList( void );

   bool Add( char * key, char * value );
   char * Search( char * key );
   char * Search( char * key, char * otherwise );
   int    Search( char * key, int otherwise );
   void Dump( void );
   
 private:
   ConfItem * first;
   ConfItem * last;
   long items; 
};



#endif // CONF_H
