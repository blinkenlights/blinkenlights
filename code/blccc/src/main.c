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

#include "config.h"

#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include <blib/blib.h>

#include "bltypes.h"

#include "blccc.h"
#include "blconfig.h"
#include "bldispatch.h"
#include "bllisten.h"
#include "blondemand.h"


static BlCcc *ccc = NULL;


static void
sigterm_handler (gint signum)
{
  sigset_t sigset;

  sigemptyset (&sigset);
  sigprocmask (SIG_SETMASK, &sigset, NULL);

  g_print ("Received signal %d, blanking and exiting ...\n", signum);

  if (ccc)
    {
      bl_ccc_kill (ccc);
      ccc = NULL;
    }

  exit (EXIT_SUCCESS);
}

static void
install_sighandlers (void)
{
  struct sigaction sa;

  /* handle SIGTERM */
  sigfillset (&sa.sa_mask);
  sa.sa_handler = sigterm_handler;
  sa.sa_flags   = SA_RESTART;
  sigaction (SIGTERM, &sa, NULL);

  /* handle SIGINT */
  sigfillset (&sa.sa_mask);
  sa.sa_handler = sigterm_handler;
  sa.sa_flags   = SA_RESTART;
  sigaction (SIGINT, &sa, NULL);

  /* ignore SIGPIPE */
  sigfillset (&sa.sa_mask);
  sa.sa_handler = SIG_IGN;
  sa.sa_flags   = SA_RESTART;
  sigaction (SIGPIPE, &sa, NULL);
}

int
main (int   argc,
      char *argv[])
{
  GMainLoop *main_loop;
  BlConfig  *config;
  BlListen  *listen;
  GError    *error        = NULL;
  gboolean   show_version = FALSE;
  gboolean   show_help    = FALSE;
  gint       i;

  g_thread_init (NULL);
  b_init ();

  config = bl_config_new ();

  for (i = 1; i < argc; i++)
    {
      if (strcmp (argv[i], "--g-fatal-warnings") == 0)
	{
          GLogLevelFlags fatal_mask;

          fatal_mask = g_log_set_always_fatal (G_LOG_FATAL_MASK);
          fatal_mask |= G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL;
          g_log_set_always_fatal (fatal_mask);
 	  argv[i] = NULL;
	}
      else if ((strcmp (argv[i], "--help") == 0) ||
               (strcmp (argv[i], "-?") == 0))
        {
          show_help = TRUE;
          argv[i] = NULL;
        }
      else if ((strcmp (argv[i], "--version") == 0) ||
               (strcmp (argv[i], "-V") == 0))
        {
          show_version = TRUE;
          argv[i] = NULL;
        }
      else if (argv[i][0] == '-')
        {
          /*
           *  anything else starting with a '-' is an error.
           */
          g_print ("\nInvalid option \"%s\"\n", argv[i]);
          show_help = TRUE;
        }
    }

  if (! show_help)
    {
      for (i = 1; i < argc; i++)
        {
          gint k;

          for (k = i; k < argc; k++)
            if (argv[k] != NULL)
              break;

          if (k > i)
            {
              gint j;

              k -= i;

              for (j = i + k; j < argc; j++)
                argv[j-k] = argv[j];

              argc -= k;
            }
        }
    }

  if (! show_version && argc < 1)
    show_help = TRUE;

  if (show_version || show_help)
    g_print ("Blinkenlights Chaos Control Center  version "VERSION"\n");

  if (show_help)
    g_print ("\nUsage: %s [options] [configfile]\n\n"
             "Options:\n"
             "  -?, --help           Output this help.\n"
             "  -V, --version        Output version information.\n"
             "  --g-fatal-warnings   Crash on warnings (for debugging)\n"
             "\n",
             argv[0]);

  if (show_version || show_help)
    return EXIT_FAILURE;

  /*  make sure the module types are registered  */
  b_module_infos_scan_dir (NULL);
  g_type_class_peek (BL_TYPE_ON_DEMAND);
  g_type_class_peek (BL_TYPE_DISPATCH);
  g_type_class_peek (B_TYPE_MOVIE_PLAYER);

  if (argc > 1 && !bl_config_parse (config, argv[1], &error))
    {
      g_printerr ("Error parsing config file '%s': %s\n",
                  argv[1], error->message);
      return EXIT_FAILURE;
    }

  if (config->channels != 1)
    {
      g_warning ("Sorry, channels != 1 is not supported.\n");
      return EXIT_FAILURE;
    }
  if (config->maxval != 255)
    {
      g_warning ("Sorry, maxval != 255 is not supported.\n");
      return EXIT_FAILURE;
    }

  g_printerr ("Loaded config '%s' with %d application(s)\n",
              b_object_get_name (B_OBJECT (config)),
              g_list_length (config->applications));

  ccc = bl_ccc_new (config);
  g_object_unref (config);

  if (!ccc)
    return EXIT_FAILURE;

  install_sighandlers ();

  listen = bl_listen_new (config->telnet_port, ccc);

  if (listen)
    {
      main_loop = g_main_loop_new (NULL, FALSE);
      g_main_loop_run (main_loop);

      g_object_unref (listen);
    }

  g_object_unref (ccc);

  return EXIT_SUCCESS;
}
