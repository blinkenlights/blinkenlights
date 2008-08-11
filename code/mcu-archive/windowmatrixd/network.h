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


#ifndef _NETWORK_H_
#define _NETWORK_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "common.h"


char        setup_socket (int *, struct sockaddr_in *);
void        close_socket (int *);
inline char get_frame    (const int, struct sockaddr_in *, bl_frame_t *);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _NETWORK_H_ */
