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

#ifndef __BL_LOGGER_H__
#define __BL_LOGGER_H__

G_BEGIN_DECLS

#define BL_TYPE_LOGGER            (bl_logger_get_type ())
#define BL_LOGGER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BL_TYPE_LOGGER, BlLogger))
#define BL_LOGGER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BL_TYPE_LOGGER, BlLoggerClass))
#define BL_IS_LOGGER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BL_TYPE_LOGGER))
#define BL_IS_LOGGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BL_TYPE_LOGGER))

typedef struct _BlLoggerClass  BlLoggerClass;

struct _BlLoggerClass
{
  BObjectClass   parent_class;
};

struct _BlLogger
{
  BObject   parent_instance;
  
  FILE     *stream;
  BWriter  *writer;

  gchar     year[8];
  gchar     month[8];
  gchar     day[8];
  gchar     hour[8];
  gchar     minute[8];
  gchar     second[8];
};


GType      bl_logger_get_type      (void) G_GNUC_CONST;
BlLogger * bl_logger_new_from_file (const gchar  *filename,
                                    GError      **error);
void       bl_logger_start_module  (BlLogger     *logger,
                                    BModule      *module);
void       bl_logger_stop          (BlLogger     *logger);

G_END_DECLS

#endif /* __BL_LOGGER_H__ */
