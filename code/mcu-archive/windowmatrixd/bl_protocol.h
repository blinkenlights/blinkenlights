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


#ifndef _BL_PROTOCOL_H_
#define _BL_PROTOCOL_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "common.h"


typedef struct {
  u_int32_t frame_magic;
  u_int32_t frame_count;
  u_int16_t frame_width;
  u_int16_t frame_height;
  u_int8_t  frame_data[HEIGHT][WIDTH];
} bl_frame_t;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _BL_PROTOCOL_H_ */
