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


#ifndef _DEVICES_H_
#define _DEVICES_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "common.h"


#define NUMBER_OF_DEVS	(WIDTH * HEIGHT / 8)


typedef struct {
  int           dev_fd;
  unsigned char dev_buffer;
} device_t;

typedef struct {
  int table_devicenum;
  int table_bitnum;
} table_t;


device_t *  setup_devices   (void);
void        close_devices   (device_t *);
inline void output_frame    (const bl_frame_t *, device_t *);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DEVICES_H_ */


