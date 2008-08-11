/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2002  The Blinkenlights Crew
 *                          Daniel Mack <daniel@yoobay.net>
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

#ifndef __B_MODULE_INFO_H__
#define __B_MODULE_INFO_H__

#include <gmodule.h>

#define B_TYPE_MODULE_INFO            (b_module_info_get_type ())
#define B_MODULE_INFO(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_MODULE_INFO, BModuleInfo))
#define B_MODULE_INFO_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_MODULE_INFO, BModuleInfoClass))
#define B_IS_MODULE_INFO(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_MODULE_INFO))
#define B_IS_MODULE_INFO_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), B_TYPE_MODULE_INFO))
#define B_MODULE_INFO_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), B_TYPE_MODULE_INFO, BModuleInfoClass))


typedef struct _BModuleInfoClass BModuleInfoClass;

struct _BModuleInfo
{
  GTypeModule  parent_instance;
  
  GModule     *module;
  gchar       *filename;

  gboolean   (* register_module) (GTypeModule *module);
};

struct _BModuleInfoClass
{
  GTypeModuleClass  parent_class;
};


GType         b_module_info_get_type  (void) G_GNUC_CONST;
BModuleInfo * b_module_info_new       (const gchar *filename);

gint          b_module_infos_scan_dir (const gchar *dirname);


#endif /* __B_MODULE_INFO_H__ */
