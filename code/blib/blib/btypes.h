/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (C) 2002  The Blinkenlights Crew
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

#ifndef __B_TYPES_H__
#define __B_TYPES_H__

G_BEGIN_DECLS

typedef enum
{
  B_KEY_0,
  B_KEY_1,
  B_KEY_2,
  B_KEY_3,
  B_KEY_4,
  B_KEY_5,
  B_KEY_6,
  B_KEY_7,
  B_KEY_8,
  B_KEY_9,
  B_KEY_HASH,
  B_KEY_ASTERISK
} BModuleKey;

typedef enum
{
  B_EVENT_TYPE_UNKNOWN = 0,
  B_EVENT_TYPE_KEY,
  B_EVENT_TYPE_PLAYER_ENTERED,
  B_EVENT_TYPE_PLAYER_LEFT
} BModuleEventType;

struct _BColor
{
  guchar  a;
  guchar  r;
  guchar  g;
  guchar  b;
};

struct _BRectangle
{
  gint    x;
  gint    y;
  gint    w;
  gint    h;
};

typedef  struct _BColor        BColor;
typedef  struct _BEffects      BEffects;
typedef  struct _BModule       BModule;
typedef  struct _BModuleEvent  BModuleEvent;
typedef  struct _BModuleInfo   BModuleInfo;
typedef  struct _BMovie        BMovie;
typedef  struct _BMoviePlayer  BMoviePlayer;
typedef  struct _BObject       BObject;
typedef  struct _BPacket       BPacket;
typedef  struct _BParser       BParser;
typedef  struct _BProxyClient  BProxyClient;
typedef  struct _BProxyServer  BProxyServer;
typedef  struct _BReceiver     BReceiver;
typedef  struct _BRectangle    BRectangle;
typedef  struct _BSender       BSender;
typedef  struct _BTheme        BTheme;
typedef  struct _BThemesQuery  BThemesQuery;
typedef  struct _BWriter       BWriter;


#define B_TYPE_FILENAME               (b_filename_get_type ())
#define B_VALUE_HOLDS_FILENAME(value) (G_TYPE_CHECK_VALUE_TYPE ((value), B_TYPE_FILENAME))

GType   b_filename_get_type           (void) G_GNUC_CONST;


G_END_DECLS

#endif  /* __B_TYPES_H__ */
