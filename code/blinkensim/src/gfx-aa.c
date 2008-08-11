/* blinkensim-aa - aalib output for Blinkenlights simulator
 *
 * Copyright (c) 2003       Steven Pickles <http://pix.test.at/>
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

#include "config.h"

#include <blib/blib-aa.h>

#include "gfx.h"

static  aa_context *context   = NULL;
static  GMainLoop  *main_loop = NULL;

gboolean
gfx_init (gint    *argc,
          gchar ***argv,
          GError **error)
{
  gint result;
  gint i;

  for( i = 0; i < *argc; i++)
    {
      if (strcmp ((*argv)[i], "-aahelp") == 0)
        {
          g_set_error (error, 0, 0, aa_help);
          return FALSE;
        }
    }

  g_printerr ("(use -aahelp to get aalib specific help)\n");

  return aa_parseoptions (NULL, NULL, argc, *argv);
}

void
gfx_close (void)
{
  if (context)
    {
      aa_close (context);
      context = NULL;
    }
}

GObject *
gfx_view_new (BTheme     *theme,
              GMainLoop  *loop,
              GError    **error)
{
  BViewAA *view;

  context = aa_autoinit (&aa_defparams);
  if (! context)
    return FALSE;

  aa_hidecursor (context);
  aa_autoinitkbd (context, 0);
  aa_resizehandler (context, (void *) aa_resize);

  view = b_view_aa_new (context,
                        theme->rows, theme->columns, theme->channels,
                        error);
  if (! view)
    return NULL;

  main_loop = loop;

  return G_OBJECT (view);
}

void
gfx_view_update (GObject      *view,
                 const guchar *data)
{
  /* this should probably use the glib event stuff,
     but i don't quite understand it yet. it's hard
     to just copy the directfb example, because aalib
     can't check for events without popping them
     out of the queue (yes, i am very lazy).

     the upshot of doing it this way is that if the
     connection dies, you have to force quit.
  */

  switch (aa_getevent (B_VIEW_AA (view)->aac, 0))
    {
    case AA_NONE:
      break;
    case AA_ESC:
    case 'q':
    case 'Q':
      g_main_loop_quit (main_loop);
      break;
    default:
      break;
    }

  b_view_aa_update (B_VIEW_AA (view), data);
}
