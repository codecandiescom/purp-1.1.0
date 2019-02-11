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
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

#include "confclass.h"
#include "conf.h"
#include "y.tab.h"


extern FILE * yyin;

Conf::Conf( void )
{
}
   
Conf::~Conf( void )
{
}
   
bool Conf::LoadFile( const char * filename )
{
   FILE * fd;

   /* (try to) open configfile */
   fd = fopen( filename, "r" );
   if( ! fd )
     {
	config_exists = false;
	return false;
     }

   /* Create our config-list */
   cl = new ConfList();

   config_exists = true;

   yyin = fd;
   yyparse();

   fclose( fd );

   return true;
}

bool Conf::Add( char * key, char * value )
{
   return cl->Add( key, value );   
}



/* Search and return match */
char * Conf::Search( char * key )
{
   if( config_exists )
     return cl->Search( key );
   else
     return "";
}


/* Search and return match or return "otherwise" */
char * Conf::Search( char * key, char * otherwise )
{
   if( config_exists )
     return cl->Search( key, otherwise );
   else
     return otherwise;
}

/* Search and return match or return "otherwise" */
int Conf::Search( char * key, int otherwise )
{
   if( config_exists )
     return cl->Search( key, otherwise );
   else
     return otherwise;
}




/* =============================================================== */
/*  ConfList - A linked-list-class for config-values               */
/* =============================================================== */
ConfList::ConfList( void )
{
   first = NULL;
   last = NULL;
   items = 0;
}


ConfList::~ConfList( void )
{
   if( items > 0 )
     {
	ConfItem * item = first;
	ConfItem * next;
	
	while( item != NULL )
	  {
	     next = item->GetNext();
	     delete item;
	     item = next;
	  }
     }
}

/* Add a string valye */
bool ConfList::Add( char * key, char * value )
{
   if( last != NULL )
     {
	last->AddAfter( new ConfItem( key, value ) );
	last->GetNext()->SetPrev( last );
	last = last->GetNext();
     }
   else
     {
	first = last = new ConfItem( key, value );
     }
   
   items++;
   return true;
}





/* Search and return match */
char * ConfList::Search( char * key )
{
   if( items > 0 )
     {
	ConfItem * item = first;
	char * result;

	while( item != NULL )
	  {
	     result = item->Match( key );
	     if( result != NULL ) return result;

	     item = item->GetNext();
	  }     
     }
   else
     return NULL;
}


/* Search and return match or return "otherwise" */
char * ConfList::Search( char * key, char * otherwise )
{
   if( items > 0 )
     {
	ConfItem * item = first;
	char * result;

	while( item != NULL )
	  {
	     result = item->Match( key );
	     if( result != NULL ) return result;

	     item = item->GetNext();
	  }     
	return otherwise;
     }
   else
     return otherwise;
}


/* Search and return match or return "otherwise" as integer */
int ConfList::Search( char * key, int otherwise )
{
   if( items > 0 )
     {
	ConfItem * item = first;
	char * result;

	while( item != NULL )
	  {
	     result = item->Match( key );
	     if( result != NULL ) return atoi(result);

	     item = item->GetNext();
	  }     
	return otherwise;
     }
   else
     return otherwise;
}


void ConfList::Dump()
{
   if( items > 0 )
     {
	ConfItem * item = first;
	char * result;

	while( item != NULL )
	  {
	     printf("Key: %s     Value: %s\n", item->GetKey(), item->GetValue() );
	     item = item->GetNext();
	  }     
     }
}


/* =============================================================== */
/*  ConfItem - A linked-list-class for config-values               */
/* =============================================================== */


ConfItem::ConfItem( char * key, char * value )
{
   next = NULL;
   prev = NULL;
   
   this->key = new char [ sizeof( key ) ];
   this->value = new char [ sizeof( value ) ];
   
   strcpy( this->key, key );
   strcpy( this->value, value );
}

ConfItem::~ConfItem( void )
{
   printf( "Destructing %s..\n", key );
   if( key != NULL )
     delete key;
   
   if( value != NULL )
     delete value;
}


/* Add an item after this item, move childrens back */
bool  ConfItem::AddAfter( ConfItem * item )
{
   this->next = item;
   return true;
}


char * ConfItem::GetKey( void )
{
   return key;
}
   
char * ConfItem::GetValue( void )
{
   return value;
}

ConfItem * ConfItem::GetNext( void )
{
   return next;
}

ConfItem * ConfItem::GetPrev( void )
{
   return prev;
}

void ConfItem::SetNext( ConfItem * item )
{
   next = item;
}

void ConfItem::SetPrev( ConfItem * item )
{
   prev = item;
}

/* Search and return match */
char * ConfItem::Match( char * key )
{
   if( ! strcmp( this->key, key ) )
     return this->value;
   else
     return NULL;
}


