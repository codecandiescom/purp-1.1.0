#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <rpm/rpmlib.h>

int main(int argc, char **argv)
{
  Header h; 
  int offset;
  const char * rootdir = "/";
  rpmdb db = NULL;
  rpmdbMatchIterator dbiter;
  char * name;
  char * version;
  char * release;
  int_32 count, type;
   
  /* Read the RPM-configs */
  if( rpmReadConfigFiles( NULL, NULL ))
    printf( "Could not read config-files!" );
  
  /* Open the RPM-database */
  if( rpmdbOpen( rootdir, &db, O_RDONLY, 0644) ) 
    printf("Couldn't open the RPM database");
  
  /* Create rpmdb-iterator */
  dbiter = rpmdbInitIterator( db, RPMTAG_NAME, "rpm", 0 );
  //  rpmdbSetIteratorVersion( dbiter, "3.0" );

  /* Loop over the results */
  while( 1 ) 
    {
      h = rpmdbNextIterator( dbiter );
      if( h == NULL ) break;

      /* Get package-information */
      headerGetEntry(h, RPMTAG_NAME, &type, (void **) &name, &count); 
      headerGetEntry(h, RPMTAG_VERSION, &type, (void **) &version, &count);
      headerGetEntry(h, RPMTAG_RELEASE, &type, (void **) &release, &count);
      printf("%s-%s-%s\n", name, version, release);
    }                                    

  /* Free the rpmdb-iterator */
  rpmdbFreeIterator( dbiter );

  /* Close the RPM-database */
  if( db ) rpmdbClose( db );
  
}



