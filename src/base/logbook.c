
#include "logbook.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
//#include <threads.h>

static const char log_filename[] = "orf_n_log.txt";
static const char *p_types[4] = { "UNSPECIFIED", "INFO", "WARNING", "ERROR" };
static FILE *logfile = NULL;
// static mtx_t log_file_mutex;

inline void logbook_log( logbook_error_t type, char *message ) {
	// shouldn't happen if assembled thoroughly, but anyway
	if( MAX_LEN_MESSAGES <= strlen(message) )
		message[MAX_LEN_MESSAGES-1] = 0;
	if( type < LOG_UNSPECIFIED || type > LOG_ERROR )
		type = LOG_UNSPECIFIED;
	if( logfile ) {
		const time_t t = time( NULL );
		char *now = ctime( &t );
		// Overwrite newline
		now[strlen(now)-1] = 0;
		char msg[MAX_LEN_MESSAGES];
		snprintf( msg, MAX_LEN_MESSAGES, "[%s] [%s] %s\n", p_types[type], now, message );
		fputs( msg, logfile );
		fflush(logfile);
		fputs( msg, stdout );
	} else
		fprintf( stderr, "Logfile not open for logging. Message '%s'", message );
}

inline void logbook_init() {
	logfile = fopen( log_filename, "w" );
	if( !logfile )
		fputs( "Error opening logfile ! Missing access/rights ?", stderr );
	else {
		fclose( logfile );
		logfile = fopen( log_filename, "a" );
	}
}

inline void logbook_de_init() {
	if( logfile )
		fclose( logfile );
}
