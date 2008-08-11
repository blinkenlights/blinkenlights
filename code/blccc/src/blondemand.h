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

#ifndef __BL_ON_DEMAND_H__
#define __BL_ON_DEMAND_H__

#define MAX_ON_DEMAND_CHARS 8

G_BEGIN_DECLS

#define BL_TYPE_ON_DEMAND            (bl_on_demand_get_type ())
#define BL_ON_DEMAND(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BL_TYPE_ON_DEMAND, BlOnDemand))
#define BL_ON_DEMAND_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BL_TYPE_ON_DEMAND, BlOnDemandClass))
#define BL_IS_ON_DEMAND(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BL_TYPE_ON_DEMAND))
#define BL_IS_ON_DEMAND_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BL_TYPE_ON_DEMAND))
#define BL_ON_DEMAND_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), BL_TYPE_ON_DEMAND, BlOnDemandClass))

typedef struct _BlOnDemandClass BlOnDemandClass;

struct _BlOnDemand
{
  BMoviePlayer       parent_instance;

  gchar             *filename;
  BlLoveletters     *loveletters;

  gboolean           advanced;

  gint               chars;
  gchar              code[MAX_ON_DEMAND_CHARS + 1];

  gboolean           movie_active;
  gboolean           movie_paused;
};

struct _BlOnDemandClass
{
  BMoviePlayerClass  parent_class;
};

GType bl_on_demand_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __BL_ON_DEMAND_H__ */
