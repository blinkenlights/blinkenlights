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


#ifndef _CONFIG_H_
#define _CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define PORT                      2323
#define WIDTH                       18
#define HEIGHT                       8
#define MAGIC               0xDEADBEEF
#define LOGNAME    "windowmatrixd.log"
#define BLANK_TIMEOUT               60 /* 60 seconds */
#define IPV6_MCASTGROUP  "ff07::17:17"
#define IPV4_MCASTGROUP "224.17.17.17"


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _CONFIG_H_ */
