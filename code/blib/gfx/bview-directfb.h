/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2004  The Blinkenlights Crew
 *                          Sven Neumann <sven@gimp.org>
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

#ifndef __B_VIEW_DIRECTFB_H__
#define __B_VIEW_DIRECTFB_H__

G_BEGIN_DECLS

#define B_TYPE_VIEW_DIRECTFB         (b_view_directfb_get_type ())
#define B_VIEW_DIRECTFB(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_VIEW_DIRECTFB, BViewDirectFB))
#define B_VIEW_DIRECTFB_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_VIEW_DIRECTFB, BViewDirectFBClass))
#define B_IS_VIEW_DIRECTFB(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_VIEW_DIRECTFB))


typedef struct _BViewDirectFBClass BViewDirectFBClass;
typedef struct _BViewDirectFB      BViewDirectFB;

struct _BViewDirectFBClass
{
  GObjectClass  parent_class;
};

struct _BViewDirectFB
{
  /*< private >*/
  GObject           parent_instance;

  BTheme           *theme;

  IDirectFBSurface *dest;
  IDirectFBSurface *background;
  GHashTable       *images;
};


GType           b_view_directfb_get_type (void) G_GNUC_CONST;
BViewDirectFB * b_view_directfb_new      (BTheme            *theme,
                                          IDirectFB         *dfb,
                                          IDirectFBSurface  *dest,
                                          GError           **error);

/* the view expects data with a maxval of 255 */
void            b_view_directfb_update   (BViewDirectFB     *view,
                                          const guchar      *frame_data);

G_END_DECLS

#endif /* __B_VIEW_DIRECTFB_H__ */
