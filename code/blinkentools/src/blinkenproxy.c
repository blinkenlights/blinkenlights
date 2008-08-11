/* blinkenproxy - forwards Blinkenlights packets
 *
 * Copyright (c) 2003-2004  Hannes Mehnert <hannes@mehnert.org>
 *                          Sven Neumann <sven@gimp.org>
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
#include <string.h>

#ifdef HAVE_GETOPT_LONG
#define _GNU_SOURCE
#include <getopt.h>
#endif

#include <blib/blib.h>

#ifdef G_OS_WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#endif


#ifdef HAVE_GETOPT_LONG
static const struct option options[] =
{
  { "help",    no_argument, NULL, '?' },
  { "version", no_argument, NULL, 'V' },
  { NULL,      0,           NULL,  0  }
};
#endif

static const gchar *option_str = "?V";


static  BProxyServer *server    = NULL;
static  GMainLoop    *loop      = NULL;
static  guint         heartbeat = 0;


#define  HEARTBEAT_TIMEOUT  (B_HEARTBEAT_INTERVAL * 12)


static void
usage (const gchar *name)
{
  g_printerr ("\n");
  g_printerr ("blinkenproxy - forwards Blinkenlights package to its clients.\n");
  g_printerr ("Check http://www.blinkenlights.de/ for more information.\n\n");

  g_printerr ("Usage: %s [options] [hostname[:port]|receive-port] [heartbeat-port]\n", name);
  g_printerr ("Options:\n");
  g_printerr ("   -?, --help       output usage information\n");
  g_printerr ("   -V, --version    output version information\n");
  g_printerr ("\n");
}

static gboolean
frame_callback (BReceiver *receiver,
                BPacket   *packet,
                gpointer   data)
{
  BProxyServer *server = B_PROXY_SERVER (data);

  if (packet->header.mcu_frame_h.magic != MAGIC_MCU_FRAME)
    return TRUE;

  b_proxy_server_send_packet (server, packet);

  return TRUE;
}

static void
server_client_added (BProxyServer *server,
                     const gchar  *host,
                     gint          port,
                     BProxyClient *client)
{
  gint num = b_proxy_server_num_clients (server);

  g_print ("client added (%d): %s:%d\n", num, host, port);

  if (client && !heartbeat)
    {
      b_proxy_client_send_heartbeat (client);

      heartbeat = g_timeout_add (B_HEARTBEAT_INTERVAL,
                                 (GSourceFunc) b_proxy_client_send_heartbeat,
                                 client);
    }
}

static void
server_client_removed (BProxyServer *server,
                       const gchar  *host,
                       gint          port,
                       BProxyClient *client)
{
  gint num = b_proxy_server_num_clients (server);

  g_print ("client removed (%d): %s:%d\n", num, host, port);

  if (client && heartbeat && num == 0)
    {
      g_source_remove (heartbeat);
      heartbeat = 0;
    }
}

#ifndef G_OS_WIN32
static void
sigusr1_handler (gint signum)
{
  if (server)
    g_print ("I have %u clients.\n", b_proxy_server_num_clients (server));
}

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
#endif  /*  !G_OS_WIN32  */


int
main (int argc,
      char *argv[])
{
  BReceiver    *receiver;
  BProxyClient *client   = NULL;
  const gchar  *host     = NULL;
  gint          bml_port = MCU_LISTENER_PORT;
  gint          hb_port  = B_HEARTBEAT_PORT;
  gint          c;
  GError       *error    = NULL;

  g_printerr ("blinkenproxy version " VERSION "\n");

  b_init ();

#ifdef HAVE_GETOPT_LONG
  while ((c = getopt_long (argc, argv, option_str, options, NULL)) >= 0)
#else
  while ((c = getopt (argc, argv, option_str)) >= 0)
#endif
    {
      switch (c)
        {
        case '?':
          usage (argv[0]);
          return EXIT_SUCCESS;

        case 'V':
          g_printerr ("blinkenproxy (%s version %s)\n", PACKAGE, VERSION);
          return EXIT_SUCCESS;

        default:
          usage (argv[0]);
          return EXIT_FAILURE;
        }
    }

  switch (argc - optind)
    {
    case 0:
      break;

    case 1:
      if (! b_parse_int (argv[optind], &bml_port))
        host = argv[optind];
      break;

    case 2:
      if (! b_parse_int (argv[optind], &bml_port))
        host = argv[optind];

      b_parse_int (argv[optind + 1], &hb_port);
      break;

    default:
      usage (argv[0]);
      return EXIT_FAILURE;
    }

  if (host)
    {
      gchar *name  = g_strdup (host);
      gchar *colon = strrchr (name, ':');

      bml_port = B_HEARTBEAT_PORT;

      if (colon)
        {
          b_parse_int (colon + 1, &bml_port);
          *colon = '\0';
        }

      host = name;
    }

  server = b_proxy_server_new (hb_port, &error);
  if (! server)
    {
      g_printerr ("Couldn't create proxy server: %s\n", error->message);
      return EXIT_FAILURE;
    }

  g_print ("Listening for client heartbeat on port %d.\n", hb_port);

  if (host)
    {
      g_print ("Forwarding stream from blinkenproxy at %s:%d\n",
               host, bml_port);

      client = b_proxy_client_new (host, bml_port, -1,
                                   frame_callback, server,
                                   &error);

      if (! client)
        {
          g_printerr ("Couldn't setup proxy connection: %s\n", error->message);
          return EXIT_FAILURE;
        }

      receiver = B_RECEIVER (client);
    }
  else
    {
      receiver = b_receiver_new (frame_callback, server);

      if (! b_receiver_listen (receiver, bml_port))
        {
          g_printerr ("Unable to listen on port %d!\n", bml_port);
          return EXIT_FAILURE;
        }

      g_print ("Listening for Blinkenlights packages on port %d.\n", bml_port);
    }

  g_signal_connect (server, "client_added",
                    G_CALLBACK (server_client_added),
                    client);
  g_signal_connect (server, "client_removed",
                    G_CALLBACK (server_client_removed),
                    client);

  loop = g_main_loop_new (NULL, FALSE);

#ifndef G_OS_WIN32
  {
    struct sigaction  sa;

    /* handle SIGINT */
    sigfillset (&sa.sa_mask);
    sa.sa_handler = sigint_handler;
    sa.sa_flags   = SA_RESTART;
    sigaction (SIGINT, &sa, NULL);

    /* handle SIGUSR1 */
    sigfillset (&sa.sa_mask);
    sa.sa_handler = sigusr1_handler;
    sa.sa_flags   = SA_RESTART;
    sigaction (SIGUSR1, &sa, NULL);
  }
#endif  /*  !G_OS_WIN32  */

  g_main_loop_run (loop);
  g_main_loop_unref (loop);

  g_object_unref (server);

  return EXIT_SUCCESS;
}
