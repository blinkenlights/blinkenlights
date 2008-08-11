/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2003       pix <http://pix.test.at/>
 * Copyright (c) 2001-2002  The Blinkenlights Crew
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

#ifndef __B_VIEW_AA_H__
#define __B_VIEW_AA_H__

G_BEGIN_DECLS

#define B_TYPE_VIEW_AA         (b_view_aa_get_type ())
#define B_VIEW_AA(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_VIEW_AA, BViewAA))
#define B_VIEW_AA_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_VIEW_AA, BViewAAClass))
#define B_IS_VIEW_AA(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_VIEW_AA))


typedef struct _BViewAAClass BViewAAClass;
typedef struct _BViewAA      BViewAA;

struct _BViewAAClass
{
  GObjectClass  parent_class;
};

struct _BViewAA
{
  /*< private >*/
  GObject       parent_instance;

  gint          rows;
  gint          columns;
  gint          channels;

  gint          char_width;
  gint          char_height;

  aa_context   *aac;
};


GType     b_view_aa_get_type (void) G_GNUC_CONST;
BViewAA * b_view_aa_new      (aa_context    *aac,
                              gint           rows,
                              gint           columns,
                              gint           channels,
			      GError       **error);

/* the view expects data with a maxval of 255 */
void      b_view_aa_update   (BViewAA       *view,
                              const guchar  *frame_data);

G_END_DECLS

#endif /* __B_VIEW_AA_H__ */
