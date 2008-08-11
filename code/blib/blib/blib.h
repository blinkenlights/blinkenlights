/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (C) 2002-2003  The Blinkenlights Crew
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

#ifndef __BLIB_H__
#define __BLIB_H__

#include <glib-object.h>

#include <blib/btypes.h>

#include <blib/beffects.h>
#include <blib/bmovie.h>
#include <blib/bmovie-blm.h>
#include <blib/bmovie-bml.h>
#include <blib/bmovie-gif.h>
#include <blib/bmovie-effects.h>
#include <blib/bobject.h>
#include <blib/bpacket.h>
#include <blib/bparams.h>
#include <blib/bparser.h>
#include <blib/bproxyclient.h>
#include <blib/bproxyserver.h>
#include <blib/breceiver.h>
#include <blib/bsender.h>
#include <blib/bsocket.h>
#include <blib/btheme.h>
#include <blib/bthemes.h>
#include <blib/butils.h>
#include <blib/bwriter.h>
#include <blib/bmodule.h>
#include <blib/bmodule-internal.h>
#include <blib/bmodule-utils.h>
#include <blib/bmoduleinfo.h>
#include <blib/bmovieplayer.h>

#include <blib/gif-load.h>
#include <blib/gif-save.h>

void  b_init (void);

#endif /* __BLIB_H__ */
