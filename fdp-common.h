// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2013-2018 ANSSI. All Rights Reserved.
#include <syslog.h>
#include <errno.h>

/* Log functions picked from userd */

#define _LOG(lev, slev, fmt, args...) do {\
	if (g_verbose >= lev) { \
		if (!g_daemonized) { \
		  printf("%s(%s:%d): "fmt"\n", __FUNCTION__,		\
			 __FILE__, __LINE__, ##args);			\
		} else {						\
		  syslog(LOG_DAEMON|slev,				\
			 "%s(%s:%d): "fmt"\n", __FUNCTION__,		\
			 __FILE__, __LINE__, ##args);			\
		} \
	} \
} while (0)

#define LOG(fmt, args...) _LOG(0, LOG_NOTICE, fmt, ##args);
#define LOG2(fmt, args...) _LOG(1, LOG_NOTICE, fmt, ##args);
#define DEBUG(fmt, args...) _LOG(0, LOG_DEBUG, fmt, ##args);

#define ERROR(fmt, args...) do {\
	if (!g_daemonized) { \
	  fprintf(stderr, "%s(%s:%d): "fmt"\n", __FUNCTION__,		\
		  __FILE__, __LINE__, ##args);				\
	} else { \
	  syslog(LOG_DAEMON|LOG_ERR, "%s(%s:%d): "fmt"\n",    \
		 __FUNCTION__,						\
		 __FILE__, __LINE__, ##args);				\
	} \
} while (0)

#define ERROR_ERRNO(fmt, args...) \
	ERROR(fmt": %s", ##args, strerror(errno))
