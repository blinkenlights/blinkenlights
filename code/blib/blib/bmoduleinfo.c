/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2002  The Blinkenlights Crew
 *                          Daniel Mack <daniel@yoobay.net>
 *                          Michael Natterer <mitch@gimp.org>
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

#include "config.h"

#include <string.h>
#include <errno.h>

#include <gmodule.h>
#include <glib-object.h>

#include "btypes.h"
#include "bmoduleinfo.h"


static void     b_module_info_class_init (BModuleInfoClass *klass);
static void     b_module_info_init       (BModuleInfo      *module_info);
static void     b_module_info_finalize   (GObject          *object);
static gboolean b_module_info_load       (GTypeModule      *gmodule);
static void     b_module_info_unload     (GTypeModule      *gmodule);


static GSList           *module_infos = NULL;
static GTypeModuleClass *parent_class = NULL;


GType
b_module_info_get_type (void)
{
  static GType module_info_type = 0;

  if (! module_info_type)
    {
      static const GTypeInfo module_info_info =
      {
        sizeof (BModuleInfoClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) b_module_info_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data     */
        sizeof (BModuleInfo),
        0,              /* n_preallocs    */
        (GInstanceInitFunc) b_module_info_init,
      };
      
      module_info_type = g_type_register_static (G_TYPE_TYPE_MODULE,
                                                 "BModuleInfo",
                                                 &module_info_info, 0);
    }
  
  return module_info_type;
}

static void
b_module_info_class_init (BModuleInfoClass *klass)
{
  GObjectClass     *object_class;
  GTypeModuleClass *module_class;
	
  object_class = G_OBJECT_CLASS (klass);
  module_class = G_TYPE_MODULE_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->finalize = b_module_info_finalize;

  module_class->load     = b_module_info_load;
  module_class->unload   = b_module_info_unload;
}

static void
b_module_info_init (BModuleInfo *module)
{
  module->module   = NULL;
  module->filename = NULL;
}

static void
b_module_info_finalize (GObject *object)
{
  BModuleInfo *module_info;

  module_info = B_MODULE_INFO (object);

  if (module_info->filename)
    {
      g_free (module_info->filename);
      module_info->filename = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
b_module_info_load (GTypeModule *gmodule)
{
  BModuleInfo *module_info;

  module_info = B_MODULE_INFO (gmodule);

  g_return_val_if_fail (module_info->filename != NULL, FALSE);

  module_info->module = g_module_open (module_info->filename,
                                       G_MODULE_BIND_LAZY);
	
  if (! module_info->module)
    {
      g_warning (g_module_error ());
      return FALSE;
    }

  if (! g_module_symbol (module_info->module, "b_module_register",
                         (gpointer *) &module_info->register_module))
    {
      g_warning (g_module_error ());
      g_module_close (module_info->module);
      module_info->module = NULL;

      return FALSE;
    }

  return module_info->register_module (gmodule);
}

static void
b_module_info_unload (GTypeModule *gmodule) 
{
  BModuleInfo *module_info;

  module_info = B_MODULE_INFO (gmodule);

  g_return_if_fail (module_info->module != NULL);

  g_module_close (module_info->module); /* FIXME: error handling */
  module_info->module = NULL;
}

/**
 * b_module_info_new:
 * @filename: the filename of the loadable module
 * 
 * Creates a new #BModuleInfo object a loadable module.
 * 
 * Return value: a newly allocated #BModuleInfo object
 **/
BModuleInfo *
b_module_info_new (const gchar *filename)
{
  BModuleInfo *module_info;

  g_return_val_if_fail (filename != NULL, NULL);
  g_return_val_if_fail (g_module_supported (), NULL);

  module_info = B_MODULE_INFO (g_object_new (B_TYPE_MODULE_INFO, NULL));

  module_info->filename = g_strdup (filename); 

  if (! b_module_info_load (G_TYPE_MODULE (module_info)))
    {
      g_object_unref (G_OBJECT (module_info));
      return NULL;
    }

  b_module_info_unload (G_TYPE_MODULE (module_info));

  return module_info;
}

/**
 * b_module_infos_scan_dir:
 * @dirname: the name of the directory to scan or %NULL to use the
 * default directory
 * 
 * Scans the given directory for loadable modules and registers the
 * types implemented by the found modules.
 * 
 * Return value: the number of successfully registered modules
 **/
gint
b_module_infos_scan_dir (const gchar *dirname)
{
  gint         count = 0;
  GDir        *dir;
  const gchar *name;
  char        *full_name;
  BModuleInfo *module_info;

  if (!dirname)
    dirname = MODULEPATH;

  dir = g_dir_open (dirname, 0, NULL);
  if (dir == NULL)
    {
      g_warning ("Unable to open dir '%s': %s", dirname, g_strerror (errno));
      return 0;
    }

  while ((name = g_dir_read_name (dir)))
    {
      if ((strlen (name) < 7)      ||
          strncmp (name, "lib", 3) ||
          strcmp (name + strlen (name) - 3, ".la"))
        continue;

      full_name = g_build_filename (dirname, name, NULL);

#ifdef VERBOSE
      g_print ("Testing file '%s'\n", full_name);
#endif

      if (g_file_test (full_name, G_FILE_TEST_IS_REGULAR))
        {
          module_info = b_module_info_new (full_name);

          if (module_info)
            {
#ifdef VERBOSE
              g_print ("Successfully loaded module '%s'\n", full_name);
#endif
              module_infos = g_slist_append (module_infos, module_info);
              count++;
            }
        }

      g_free (full_name);
    }

  g_dir_close (dir);

  return count;
}
