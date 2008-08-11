/* blinkensim - a Blinkenlights simulator
 *
 * Copyright (c) 2001-2004  Sven Neumann <sven@gimp.org>
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

#include <unistd.h>
#include <string.h>

#ifdef HAVE_GETOPT_LONG
#define _GNU_SOURCE
#include <getopt.h>
#endif

#include <blib/blib.h>

#ifndef G_OS_WIN32
#include <signal.h>
#endif

#include "gfx.h"


static  GObject   *view = NULL;
static  GMainLoop *loop = NULL;
static  guchar     lut[256];


#ifdef HAVE_GETOPT_LONG
static const struct option options[] =
{
  { "list",    no_argument,       NULL, 'l' },
  { "port",    required_argument, NULL, 'p' },
  { "verbose", no_argument,       NULL, 'v' },
  { "help",    no_argument,       NULL, '?' },
  { "version", no_argument,       NULL, 'V' },
  { NULL,      0,                 NULL,  0  }
};
#endif

static const gchar *option_str = "lp:v?V";


static void
usage (const gchar *name)
{
  g_printerr ("blinkensim - a Blinkenlights simulator.\n");
  g_printerr ("Check http://www.blinkenlights.de/ for more information.\n\n");
  g_printerr ("Usage: %s [options] <theme> [host[:port]]\n", name);
  g_printerr ("  Use host[:port] to request packets from a blinkenproxy\n");
  g_printerr ("Options:\n");
  g_printerr ("   -l, --list                 list available themes and quit\n");
  g_printerr ("   -p, --port=<port>          local UDP port to listen to\n");
  g_printerr ("   -v, --verbose              be verbose\n");
  g_printerr ("   -?, --help                 output usage information\n");
  g_printerr ("   -V, --version              output version information\n");
  g_printerr ("\n");

#ifndef HAVE_GETOPT_LONG
  g_printerr ("This version of blinkensim was compiled without support for long command-line\n");
  g_printerr ("options. Only the short, one-letter options will work.\n\n");
#endif
}

static gboolean
frame_callback (BReceiver *receiver,
                BPacket   *packet,
                gpointer   data)
{
  BTheme *theme = (BTheme *) data;

  if (packet->header.mcu_frame_h.width    != theme->columns ||
      packet->header.mcu_frame_h.height   != theme->rows    ||
      packet->header.mcu_frame_h.channels != 1)
    {
      /*  g_printerr ("Packet mismatch\n");  */
      return TRUE;
    }

  if (theme->maxval != 0xFF || packet->header.mcu_frame_h.maxval != 0xFF)
    {
      gint    n = (packet->header.mcu_frame_h.width *
                   packet->header.mcu_frame_h.height);
      guchar *d = packet->data;

      switch (packet->header.mcu_frame_h.maxval)
        {
        case 0xFF:
          while (n--)
            *d = lut[*d], d++;
          break;

        default:
          while (n--)
            {
              *d = lut[((guint)(*d) * 0xFF) /
                       packet->header.mcu_frame_h.maxval];
              d++;
            }
          break;
        }
    }

  if (view)
    gfx_view_update (view, packet->data);

  return TRUE;
}

static void
setup_lut (BTheme *theme)
{
  gint    i, d;
  gdouble v;

  g_assert (theme->maxval > 0);

  for (i = 0, d = 0, v = 0.0; i < 256; i++)
    {
      lut[i] = v + 0.5;

      d += theme->maxval + 1;
      if (d > 255)
        {
          d -= 256;
          v += 255.0 / theme->maxval;
        }
    }
}

static void
print_theme_info (const gchar *name,
                  BTheme      *theme,
                  gpointer     user_data)
{
  if (!name || !theme)
    return;

  if (theme->maxval > 1)
    g_print ("  %-20s  %s (%dx%d, %d levels) on %dx%d\n",
             name,
             b_object_get_name (B_OBJECT (theme)),
             theme->columns, theme->rows, theme->maxval + 1,
             theme->width, theme->height);
  else
    g_print ("  %-20s  %s (%dx%d) on %dx%d\n",
             name,
             b_object_get_name (B_OBJECT (theme)),
             theme->columns, theme->rows, theme->width, theme->height);
}

#ifndef G_OS_WIN32
static void
sigint_handler (gint signum)
{
  sigset_t sigset;

  sigemptyset (&sigset);
  sigprocmask (SIG_SETMASK, &sigset, NULL);

  g_print ("Received signal %d, exiting ...\n", signum);

  if (loop)
    g_main_loop_quit (loop);
}
#endif

int
main (int    argc,
      char **argv)
{
  BReceiver        *receiver;
  BTheme           *theme;
  GError           *error = NULL;
  gchar            *theme_name   = NULL;
  gchar            *proxy_host   = NULL;
  gint              proxy_port   = B_HEARTBEAT_PORT;
  gint              bml_port     = MCU_LISTENER_PORT;
  gboolean          bml_port_set = FALSE;
  gboolean          verbose      = FALSE;
  gint              c;

  g_printerr ("blinkensim version " VERSION "\n");

  if (!gfx_init (&argc, &argv, &error))
    {
      g_printerr ("%s\n", error->message);
      return EXIT_FAILURE;
    }

  b_init ();

#ifdef HAVE_GETOPT_LONG
  while ((c = getopt_long (argc, argv, option_str, options, NULL)) >= 0)
#else
  while ((c = getopt (argc, argv, option_str)) >= 0)
#endif
    {
      switch (c)
        {
        case 'l':
          g_print ("\nAvailable themes:\n");
          b_themes_foreach_theme (NULL, (GHFunc) print_theme_info, NULL);
          g_print ("\n");
          return EXIT_SUCCESS;

        case 'p':
          if (b_parse_int (optarg, &bml_port) && bml_port > 0)
            {
              bml_port_set = TRUE;
            }
          else
            {
              g_printerr ("Invalid argument (port)\n");
              return EXIT_FAILURE;
            }
          break;

        case 'v':
          verbose = TRUE;
          break;

        case '?':
          usage (argv[0]);
          return EXIT_SUCCESS;

        case 'V':
          g_printerr ("blinkensim (version %s)\n", PACKAGE, VERSION);
          return EXIT_SUCCESS;

        default:
          usage (argv[0]);
          return EXIT_FAILURE;
        }
    }

  if (optind < argc)
    {
      theme_name = argv[optind];
      optind++;
    }
  else
    {
      usage (argv[0]);
      return EXIT_FAILURE;
    }

  if (optind < argc)
    {
      gchar *arg   = argv[optind];
      gchar *colon = strrchr (arg, ':');

      if (colon)
        {
          b_parse_int (colon + 1, &proxy_port);
          *colon = '\0';
        }

      proxy_host = arg;
    }

  if (g_file_test (theme_name, G_FILE_TEST_IS_REGULAR))
    {
      theme = b_theme_new_from_file (theme_name, TRUE, &error);
      if (!theme)
        {
          g_printerr ("Error opening '%s': %s\n", theme_name, error->message);
          return EXIT_FAILURE;
        }
    }
  else
    {
      theme = b_themes_lookup_theme (theme_name, NULL, &error);
      if (!theme)
        {
          g_printerr ("\n%s\n", error->message);
          g_printerr ("Fix your spelling or try setting the "
                      "B_THEME_PATH environment variable.\n\n");
          g_printerr ("Use '%s --list' to get a list of available themes.\n",
                      argv[0]);
          return EXIT_FAILURE;
        }
      if (!b_theme_load (theme, &error))
        {
          g_printerr ("Error loading theme '%s': %s\n",
                      b_object_get_name (B_OBJECT (theme)), error->message);
          return EXIT_FAILURE;
        }
    }

  if (verbose)
    g_print ("Using theme '%s' (%dx%d, %d levels) at size %dx%d\n",
             b_object_get_name (B_OBJECT (theme)),
             theme->columns, theme->rows, theme->maxval + 1,
             theme->width, theme->height);

  setup_lut (theme);

  loop = g_main_loop_new (NULL, FALSE);

  view = gfx_view_new (theme, loop, &error);

  if (!view)
    {
      g_printerr ("%s\n", error->message);
      gfx_close ();
      return EXIT_FAILURE;
    }

  g_object_add_weak_pointer (view, (gpointer *) &view);

  if (proxy_host)
    {
      BProxyClient *client;

      if (verbose)
        g_print ("Requesting stream from blinkenproxy at %s:%d\n",
                 proxy_host, proxy_port);

      client = b_proxy_client_new (proxy_host, proxy_port,
                                   bml_port_set ? bml_port : -1,
                                   frame_callback, theme,
                                   &error);
      if (! client)
        {
          g_printerr ("Couldn't setup proxy connection: %s\n", error->message);
          return EXIT_FAILURE;
        }

      b_proxy_client_send_heartbeat (client);

      g_timeout_add (B_HEARTBEAT_INTERVAL,
                     (GSourceFunc) b_proxy_client_send_heartbeat, client);

      receiver = B_RECEIVER (client);
    }
  else
    {
      receiver = b_receiver_new (frame_callback, theme);

      b_receiver_listen (receiver, bml_port);

      if (verbose)
        g_print ("Listening on port %i for Blinkenlights packets\n", bml_port);
    }

#ifndef G_OS_WIN32
  {
    struct sigaction  sa;

    /* handle SIGINT */
    sigfillset (&sa.sa_mask);
    sa.sa_handler = sigint_handler;
    sa.sa_flags   = SA_RESTART;
    sigaction (SIGINT, &sa, NULL);
  }
#endif

  g_main_loop_run (loop);

  g_main_loop_unref (loop);

  g_object_unref (receiver);
  g_object_unref (theme);

  gfx_close ();

  return EXIT_SUCCESS;
}

