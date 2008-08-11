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


#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <libgen.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <syslog.h>

#ifdef HAVE_BMC_PIO_H
#include <bmc_pio.h>
#endif


typedef unsigned char bool;


#ifndef EXIT_FAILURE
# define EXIT_FAILURE 1
#endif /* EXIT_FAILURE */

#ifndef EXIT_SUCCESS
# define EXIT_SUCCESS 0
#endif /* EXIT_SUCCESS */

#ifndef TRUE
# define TRUE 1
#endif /* TRUE */

#ifndef FALSE
# define FALSE 0
#endif /* FALSE */


#include "config.h"
#include "bl_protocol.h"
#include "network.h"
#include "devices.h"
#include "log.h"
#include "table.h"


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _COMMON_H_ */
