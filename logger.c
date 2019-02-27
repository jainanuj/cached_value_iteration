
#include "logger.h"

/* where we log information */
FILE *logfile = NULL;

/*
 * ============================================================================
 */

void wlog( int level, const char *format, ... ) {
  va_list ap;

  va_start( ap, format );

  if ( logfile == NULL ) {
    fprintf( stderr, "LOG FILE NOT YET OPEN!\n" );
    vfprintf( stderr, format, ap );    

  } else {
    vfprintf( logfile, format, ap );
  }

  va_end( ap );

#ifdef LOGGER_ALWAYS_FLUSH
  fflush( logfile );
#endif

}

int open_logfile_stdout( void ) {
  logfile = stdout;
  return 1;
}

/*
 * ----------------------------------------------------------------------------
 */

void wlog_flush( void ) {
  fflush( logfile );
}

/*
 * ----------------------------------------------------------------------------
 */
