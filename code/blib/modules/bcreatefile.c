/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2002  The Blinkenlights Crew
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

#include <stdlib.h>
#include <string.h>

#include <glib-object.h>

#include <blib/blib.h>

#define CREATE_FILE_DIR "/var/blink/createfile"

typedef struct _BChar BChar;

#define B_TYPE_CREATEFILE            (b_type_createfile)
#define B_CREATEFILE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_CREATEFILE, BCreateFile))
#define B_CREATEFILE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_CREATEFILE, BCreateFileClass))
#define B_IS_CREATEFILE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_CREATEFILE))
#define B_IS_CREATEFILE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), B_TYPE_CREATEFILE))

typedef struct _BCreateFile      BCreateFile;
typedef struct _BCreateFileClass BCreateFileClass;

struct _BCreateFile
{
  BModule       parent_instance;
};

struct _BCreateFileClass
{
  BModuleClass  parent_class;
};


static GType      b_createfile_get_type      (GTypeModule   *module);

static void       b_createfile_class_init    (BCreateFileClass    *klass);
static void       b_createfile_init          (BCreateFile         *text);

static void       b_createfile_finalize      (GObject       *object);
static gboolean   b_createfile_query         (gint           width,
                                        gint           height,
                                        gint           channels,
                                        gint           maxval);
static gboolean   b_createfile_prepare       (BModule       *module,
                                        GError       **error);
static void       b_createfile_relax         (BModule       *module);
static void       b_createfile_start         (BModule       *module);
static void       b_createfile_stop          (BModule       *module);
static void       b_createfile_event         (BModule       *module,
                                        BModuleEvent  *event);
static void       b_createfile_describe      (BModule       *module,
                                        const gchar  **title,
                                        const gchar  **description,
                                        const gchar  **author);


static BModuleClass * parent_class = NULL;
static GType          b_type_createfile  = 0;


G_MODULE_EXPORT gboolean
b_module_register (GTypeModule *module)
{
  b_createfile_get_type (module);
  return TRUE;
}

static GType
b_createfile_get_type (GTypeModule *module)
{
  if (! b_type_createfile)
    {
      static const GTypeInfo text_info =
      {
        sizeof (BCreateFileClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_createfile_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BCreateFile),
        0,              /* n_preallocs */
        (GInstanceInitFunc) b_createfile_init,
      };

      b_type_createfile = g_type_module_register_type (module,
                                                 B_TYPE_MODULE, "BCreateFile",
                                                 &text_info, 0);
    }

  return b_type_createfile;
}

static void
b_createfile_class_init (BCreateFileClass *klass)
{
  GObjectClass *object_class;
  BModuleClass *module_class;

  object_class = G_OBJECT_CLASS (klass);
  module_class = B_MODULE_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->finalize     = b_createfile_finalize;

  module_class->max_players = 1;

  module_class->query    = b_createfile_query;
  module_class->prepare  = b_createfile_prepare;
  module_class->relax    = b_createfile_relax;
  module_class->start    = b_createfile_start;
  module_class->stop     = b_createfile_stop;
  module_class->event    = b_createfile_event;
  module_class->describe = b_createfile_describe;
}

static void
b_createfile_init (BCreateFile *text)
{
}

static void
b_createfile_finalize (GObject *object)
{
  BCreateFile *text;

  text = B_CREATEFILE (object);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
b_createfile_query (gint     width,
              gint     height,
              gint     channels,
              gint     maxval)
{
  return (width  >= 1 &&
          height >= 1 &&
          channels >= 1 &&
          maxval >= 1);
}

static gboolean
b_createfile_prepare (BModule  *module,
                GError  **error)
{
  BCreateFile *text;

  text = B_CREATEFILE (module);

  return TRUE;
}

static void
b_createfile_relax (BModule *module)
{
}

static void
b_createfile_start (BModule *module)
{
  b_module_fill (module, 0);
  b_module_paint (module);
}

static void
b_createfile_stop (BModule *module)
{
}

static void
b_createfile_event (BModule      *module,
              BModuleEvent *event)
{
  switch (event->type)
    {
    case B_EVENT_TYPE_KEY:
      switch (event->key)
        {
        default:
          case B_KEY_1:
            fclose (fopen (CREATE_FILE_DIR"/1", "wt"));
            break;
          case B_KEY_2:
            fclose (fopen (CREATE_FILE_DIR"/2", "wt"));
            break;
          case B_KEY_3:
            fclose (fopen (CREATE_FILE_DIR"/3", "wt"));
            break;
          case B_KEY_4:
            fclose (fopen (CREATE_FILE_DIR"/4", "wt"));
            break;
          case B_KEY_5:
            fclose (fopen (CREATE_FILE_DIR"/5", "wt"));
            break;
          case B_KEY_6:
            fclose (fopen (CREATE_FILE_DIR"/6", "wt"));
            break;
          case B_KEY_7:
            fclose (fopen (CREATE_FILE_DIR"/7", "wt"));
            break;
          case B_KEY_8:
            fclose (fopen (CREATE_FILE_DIR"/8", "wt"));
            break;
          case B_KEY_9:
            fclose (fopen (CREATE_FILE_DIR"/9", "wt"));
            break;
          case B_KEY_0:
            fclose (fopen (CREATE_FILE_DIR"/0", "wt"));
            break;
        }
      break;

    default:
      break;
    }
}

static void
b_createfile_describe (BModule      *module,
                 const gchar **title,
                 const gchar **description,
                 const gchar **author)
{
  *title       = "BCreateFile";
  *description = "create a file";
  *author      = "1stein";
}
