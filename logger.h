#ifndef _LOGGER_H
#define _LOGGER_H

#include <stdio.h>
#include <stdarg.h>

void wlog( int level, const char *format, ... );

int open_logfile( char *fn );
int open_logfile_default( void );
int open_logfile_stdout( void );

void wlog_flush( void );

#endif
