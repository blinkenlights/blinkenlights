/* blccc - Blinkenlights Chaos Control Center
 *
 * Copyright (c) 2001-2002  Sven Neumann <sven@gimp.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __BL_TYPES_H__
#define __BL_TYPES_H__

G_BEGIN_DECLS


typedef struct _BlApp          BlApp;
typedef struct _BlCcc          BlCcc;
typedef struct _BlConfig       BlConfig;
typedef struct _BlDispatch     BlDispatch;
typedef struct _BlEffects      BlEffects;
typedef struct _BlIsdn         BlIsdn;
typedef struct _BlIsdnLine     BlIsdnLine;
typedef struct _BlListen       BlListen;
typedef struct _BlLoveletters  BlLoveletters;
typedef struct _BlLovemodules  BlLovemodules;
typedef struct _BlLogger       BlLogger;
typedef struct _BlOnDemand     BlOnDemand;
typedef struct _BlPlaylist     BlPlaylist;
typedef struct _BLPlaylistItem BlPlaylistItem;
typedef struct _BlTheater      BlTheater;

G_END_DECLS

#endif /* __BL_TYPES_H__ */
