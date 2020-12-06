
#pragma once

#include <stdarg.h>
#include "base/base.h"	// to have MAX_LEN_MESSAGES available

typedef enum {
		LOG_UNSPECIFIED, LOG_INFO, LOG_WARNING, LOG_ERROR
} logbook_error_t;

// @todo: make thread safe
extern void logbook_log( logbook_error_t type, char *message );

extern void logbook_init();

extern void logbook_de_init();
