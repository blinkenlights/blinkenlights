/* blinkensim - a Blinkenlights simulator
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

#ifndef __GFX_H__
#define __GFX_H__

gboolean  gfx_init        (gint          *argc,
                           gchar       ***argv,
                           GError       **error);
void      gfx_close       (void);
GObject * gfx_view_new    (BTheme        *theme,
                           GMainLoop     *loop,
                           GError       **error);
void      gfx_view_update (GObject       *view,
                           const guchar  *data);

#endif /* __GFX_H__ */


