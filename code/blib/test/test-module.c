/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (C) 2002  The Blinkenlights Crew
 *                     Sven Neumann <sven@gimp.org>
 *                     Daniel Mack <daniel@yoobay.net>
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

#define GTK_DISABLE_DEPRECATED

#include <gtk/gtk.h>

#include "blib/blib.h"

/* the HDL setup */
#define WIDTH  18
#define HEIGHT 8
#define ASPECT 0.55


static GtkWidget *main_window  = NULL;
static GtkWidget *keys_table   = NULL;
static GtkWidget *start_button = NULL;
static GtkWidget *stop_button  = NULL;


static gboolean
paint (BModule  *module,
       guchar   *buffer,
       gpointer  data)
{
  b_sender_send_frame (B_SENDER (data), buffer);

  return TRUE;
}

static void
key_pressed (GObject  *object,
             gpointer  data)
{
  BModuleEvent event;

  event.device_id = 0;
  event.type      = B_EVENT_TYPE_KEY;
  event.key       = GPOINTER_TO_INT (g_object_get_data (object, "keyval"));

  g_print ("KEY %d\n", event.key);

  b_module_event (B_MODULE (data), &event);
}

static void
about_cb (GtkWidget *widget,
          BModule   *module)
{
  static GtkWidget *dialog = NULL;

  gchar   *desc[3];
  GString *text;
  gint     i;

  if (dialog)
    {
      gtk_widget_destroy (dialog);
      return;
    }

  b_module_describe (module, desc, desc + 1, desc + 2);

  text = g_string_new (NULL);

  for (i = 0; i < 3; i++)
    {
      if (i == 0)
        g_string_append_printf (text,
                                "<span weight=\"bold\" size=\"larger\">"
                                "%s"
                                "</span>", desc[0]);
      else if (desc[i])
        g_string_append (text, desc[i]);

      g_string_append_c (text, '\n');

      g_free (desc[i]);
    }

  dialog = gtk_message_dialog_new (GTK_WINDOW (main_window),
                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                   GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE,
                                   text->str);

  g_object_add_weak_pointer (G_OBJECT (dialog), (gpointer) &dialog);

  g_string_free (text, TRUE);

  gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
  gtk_window_set_title (GTK_WINDOW (dialog), "About this Module");

  gtk_label_set_use_markup (GTK_LABEL (GTK_MESSAGE_DIALOG (dialog)->label),
                            TRUE);

  g_signal_connect_swapped (G_OBJECT (dialog), "response",
                            G_CALLBACK (gtk_widget_destroy), dialog);

  gtk_widget_show (dialog);
}

static void
start_cb (gpointer data)
{
  BModuleEvent ev;

  gtk_widget_set_sensitive (keys_table,   TRUE);
  gtk_widget_set_sensitive (start_button, FALSE);
  gtk_widget_set_sensitive (stop_button,  TRUE);

  g_print ("Telling module device 0 joined...");
  ev.device_id = 0;
  ev.type = B_EVENT_TYPE_PLAYER_ENTERED;
  b_module_event (B_MODULE (data), &ev);
  g_print ("OK.\n");

}

static void
stop_cb (gpointer data)
{
  gtk_widget_set_sensitive (keys_table,   FALSE);
  gtk_widget_set_sensitive (start_button, TRUE);
  gtk_widget_set_sensitive (stop_button,  FALSE);
}

static void
dump_cb (GtkWidget *widget,
         BModule   *module)
{
  BMovie *movie;
  GError *error = NULL;

  movie = g_object_new (B_TYPE_MOVIE_BML, "name", "Module Frame Dump", NULL);

  movie->width    = module->width;
  movie->height   = module->height;
  movie->channels = module->channels;
  movie->maxval   = module->maxval;

  /* eeque */
  movie->load_count = 1;

  b_movie_prepend_frame (movie, 1000, module->buffer);

  if (!b_movie_save (movie, stdout, &error))
    {
      g_printerr ("Error writing to stdout: %s\n", error->message);
      g_clear_error (&error);
    }

  g_object_unref (movie);
}

static void
setup_window (BModule *module)
{
  GtkWidget  *main_vbox;
  GtkWidget  *vbox;
  GtkWidget  *button;
  gint        i;
  gchar      *key[]    = { "_1", "_2", "_3", "_4", "_5", "_6",
                           "_7", "_8", "_9", "_*", "_0", "_#" };
  BModuleKey  keyval[] = { B_KEY_1,
                           B_KEY_2,
                           B_KEY_3,
                           B_KEY_4,
                           B_KEY_5,
                           B_KEY_6,
                           B_KEY_7,
                           B_KEY_8,
                           B_KEY_9,
                           B_KEY_ASTERISK,
                           B_KEY_0,
                           B_KEY_HASH };

  main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_window_set_title (GTK_WINDOW (main_window), "Virtual Cellular Phone");

  g_signal_connect (G_OBJECT (main_window), "destroy",
                    G_CALLBACK (gtk_main_quit),
                    NULL);

  main_vbox = gtk_vbox_new (FALSE, 3);
  gtk_container_add (GTK_CONTAINER (main_window), main_vbox);

  button = gtk_button_new_with_mnemonic ("_About");
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (about_cb), module);
  gtk_box_pack_start (GTK_BOX (main_vbox), button, FALSE, FALSE, 0);

  keys_table = gtk_table_new (4, 3, TRUE);
  gtk_box_pack_start (GTK_BOX (main_vbox), keys_table, TRUE, TRUE, 0);

  for (i = 0; i < 12; i++)
    {
      GtkWidget *button = gtk_button_new_with_mnemonic (key[i]);

      gtk_misc_set_padding (GTK_MISC (GTK_BIN (button)->child), 8, 4);

      g_object_set_data (G_OBJECT (button),
                         "keyval", GINT_TO_POINTER (keyval[i]));

      g_signal_connect (G_OBJECT (button), "clicked",
                        G_CALLBACK (key_pressed),
                        module);
      gtk_table_attach_defaults (GTK_TABLE (keys_table), button,
                                 i%3, i%3 + 1, i/3, i/3 + 1);
    }

  gtk_widget_set_sensitive (keys_table, FALSE);

  vbox = gtk_vbox_new (TRUE, 2);
  gtk_box_pack_start (GTK_BOX (main_vbox), vbox, FALSE, FALSE, 0);

  start_button = gtk_button_new_with_mnemonic ("_Start");
  g_signal_connect_swapped (G_OBJECT (start_button), "clicked",
                            G_CALLBACK (b_module_start), module);
  gtk_box_pack_start_defaults (GTK_BOX (vbox), start_button);

  stop_button = gtk_button_new_from_stock (GTK_STOCK_STOP);
  g_signal_connect_swapped (G_OBJECT (stop_button), "clicked",
                            G_CALLBACK (b_module_stop), module);
  gtk_box_pack_start_defaults (GTK_BOX (vbox), stop_button);

  button = gtk_button_new_with_mnemonic ("_Dump Frame");
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (dump_cb), module);
  gtk_box_pack_start_defaults (GTK_BOX (vbox), button);

  gtk_widget_set_sensitive (stop_button, FALSE);

  gtk_widget_show_all (main_window);
}

int
main (int   argc,
      char *argv[])
{
  BModule *module;
  BSender *sender;
  GError  *error = NULL;
  GType    type;
  gint     i;
  gint     nreceivers = 0;

  if (argc < 4)
    {
      g_printerr ("Usage: %s <dirname> <modulename> [<host>|--property:<key>=<value>]+\n", argv[0]);
      return EXIT_FAILURE;
    }

  b_init ();

  gtk_init (&argc, &argv);

  i = b_module_infos_scan_dir (argv[1]);
  g_print ("b_module_infos_scan_dir() returned %d\n", i);

  /* make sure (static) BMoviePlayer is correctly registered. */
  g_type_class_peek (B_TYPE_MOVIE_PLAYER);

  if (i > 0)
    {
      GType *child_types;
      guint  n_child_types;

      child_types = g_type_children (B_TYPE_MODULE, &n_child_types);

      for (i = 0; i < n_child_types; i++)
        {
          g_print ("Registered BModule subtype: %s\n",
                   g_type_name (child_types[i]));
        }

      g_free (child_types);
    }


  g_print ("===================\n");
  g_print ("Getting type for '%s'\n", argv[2]);

  type = g_type_from_name (argv[2]);
  if (!type)
    {
      g_print (" ...failed.\n");
      return EXIT_FAILURE;
    }
  g_print (" ...succeeded.\n");


  g_print ("Creating BSender, ");
  sender = b_sender_new ();

  for (i = 3; i < argc; i++) {
    if (strncmp(argv[i], "--", 2) == 0)
      continue;

    b_sender_add_recipient (sender, -1, argv[i], MCU_LISTENER_PORT, NULL);
    nreceivers ++;
  }

  g_print (" added %d recipients.\n", nreceivers);

  b_sender_configure (sender, WIDTH, HEIGHT, 1, 255, MAGIC_MCU_FRAME);


  g_print ("Creating object of type '%s' ... ", argv[2]);

  module = b_module_new (type, WIDTH, HEIGHT, NULL, paint, sender, &error);

  if (! module)
    {
      g_print ("failed: %s\n", error->message);
      return EXIT_FAILURE;
    }
  g_print ("OK.\n");

  b_module_set_aspect (module, ASPECT);

  g_print ("Checking for module properties ...\n");
  for (i = 3; i < argc; i++)
    {
      const gchar *p = argv[i];
      gchar *value;
      gchar  buffer[128];

      if (strncmp (p, "--property:", 11) != 0)
        continue;

      strncpy (buffer, p + 11, 128);
      value = strchr (buffer, '=');
      if (value == NULL )
        {
          g_print ("%s seems not to be a valid property argument, ignoring\n",
                   argv[i]);
          continue;
        }

      *value = 0; /* null-terminate key */
      value++;

      g_print ("Setting object property: \"%s\" = \"%s\"\n", buffer, value);

      if (!b_object_set_property (G_OBJECT (module),
                                  buffer, value, NULL, &error))
        {
          g_print ("Failed: %s\n", error->message);
          g_clear_error (&error);
        }
    }

  g_print ("Preparing the module ... ");

  if (! b_module_prepare (module, &error))
    {
      g_print ("failed: %s\n", error->message);

      g_object_unref (module);

      return EXIT_FAILURE;
    }
  g_print ("OK.\n");


  setup_window (module);

  g_signal_connect (G_OBJECT (module), "start", G_CALLBACK (start_cb), module);
  g_signal_connect (G_OBJECT (module), "stop",  G_CALLBACK (stop_cb),  module);

  gtk_main ();

  g_print ("Quitting.\n");

  g_object_unref (module);

  return EXIT_SUCCESS;
}
