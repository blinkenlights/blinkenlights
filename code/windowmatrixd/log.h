/*
   windowmatrixd - receives blinkenframes from the network

   Copyright (C) 2001, 2002 Sebastian Klemke

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/


#ifndef _LOG_H_
#define _LOG_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "common.h"


/* This function opens the logfile. You may still use log_printf if it
fails, but its output might not be shown */
bool setup_log (const short interactive, const char  * program_name);

/* This function closes the logfile. */
void close_log (void);

/* This function printf's to either syslog or to stderr, depending on
interactivity */
void log_printf (const int level, const char * format, ...);

/* printfs to stdout when running interactively */
int interactive_printf (const char * format, ...);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _LOG_H_ */
