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

#ifndef __BL_CONFIG_H__
#define __BL_CONFIG_H__

G_BEGIN_DECLS

#define BL_TYPE_CONFIG            (bl_config_get_type ())
#define BL_CONFIG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BL_TYPE_CONFIG, BlConfig))
#define BL_CONFIG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BL_TYPE_CONFIG, BlConfigClass))
#define BL_IS_CONFIG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BL_TYPE_CONFIG))
#define BL_IS_CONFIG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BL_TYPE_CONFIG))

typedef struct _BlConfigClass  BlConfigClass;

struct _BlConfigClass
{
  BObjectClass   parent_class;
};

struct _BlConfig
{
  BObject        parent_instance;

  gint           width;
  gint           height;
  gint           maxval;
  gint           channels;
  gdouble        aspect;

  gchar         *playlist;
  gchar         *logfile;

  GList         *recipients;

  gint           telnet_port;

  gchar         *isdn_host;
  gint           isdn_port;
  gint           isdn_listen;
  gint           isdn_lines;

  GList         *authorized_callers;

  GList         *applications;
};


GType      bl_config_get_type          (void) G_GNUC_CONST;
BlConfig * bl_config_new               (void);
gboolean   bl_config_parse             (BlConfig     *config,
                                        const gchar  *filename,
                                        GError      **error);

BlApp    * bl_config_select_app        (BlConfig     *config,
                                        const gchar  *called_number);
gboolean   bl_config_authorize_caller  (BlConfig     *config,
                                        const gchar  *caller);

G_END_DECLS

#endif /* __BL_CONFIG_H__ */
