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


#include "common.h"


/* local variables */

static const char rcsid[] = "$Id: log.c,v 1.7 2002/02/27 22:07:21 packet Exp $";

static short   log_interactive = 1;
static char  * log_progname    = NULL;


bool
setup_log (const short interactive, const char  * program_name)
{
  char * tmp = NULL;

  if (log_progname)
    return (0);

  if (!interactive) {
    openlog(program_name, 0, LOG_DAEMON);
    /* FIXME: This value should also work if running in foreground and should
       be provided by user */
//    setlogmask (LOG_UPTO (LOG_NOTICE));
  }

  if (!(tmp = strdup (program_name)))
    return (0);

  log_progname    = tmp;
  log_interactive = interactive;

  return (1);
}


void
close_log (void)
{
  char * tmp = NULL;

  if (!log_interactive) {
    log_interactive = 1;
    closelog ();
  }

  if (log_progname) {
    tmp = log_progname;
    log_progname = NULL;
    free (tmp);
  }

  return;
}


void
log_printf (const int level, const char * format, ...)
{
  va_list           arglist;
  char            * tmp     = NULL;
  struct timeval    tv;

  va_start (arglist, format);
  if (log_interactive) {
    if (!(gettimeofday (&tv, NULL))) {
      if ((tmp = ctime (((time_t *) (&(tv.tv_sec)))))) {
	tmp[strlen (tmp) - 1] = '\0';
	fprintf (stderr, "[%s] ", tmp);
      }
    }
    if (log_progname)
      fprintf (stderr, "%s: ", log_progname);
    vfprintf (stderr, format, arglist);
    fprintf (stderr, "\n");
  }
  else {
    vsyslog (level, format, arglist);
  }
  va_end (arglist);

  return;
}


int
interactive_printf (const char * format, ...)
{
  va_list arglist;
  int     result = 0;

  if (log_interactive) {
    va_start (arglist, format);
    result = vfprintf (stderr, format, arglist);
    va_end (arglist);
  }

  return (result);
}
