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

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>

#include <blib/blib.h>

#include "bltypes.h"
#include "blconfig.h"
#include "blmarshal.h"
#include "blisdn.h"


/* #define ISDN_VERBOSE 1 */


#define POLL_TIMEOUT  120  /* ms  */
#define RESET_TIMEOUT  60  /* sec */


static void      bl_isdn_class_init      (BlIsdnClass  *class);
static void      bl_isdn_init            (BlIsdn       *isdn);
static void      bl_isdn_finalize        (GObject      *object);

static gboolean  bl_isdn_setup           (BlIsdn       *isdn,
                                          GError      **error);
static void      bl_isdn_teardown        (BlIsdn       *isdn);

static void      bl_isdn_hangup_all      (BlIsdn       *isdn);

static void      bl_isdn_heartbeat_start (BlIsdn       *isdn);
static void      bl_isdn_heartbeat_stop  (BlIsdn       *isdn);

static void      bl_isdn_handle_message  (BlIsdn       *isdn,
                                          const gchar  *msg,
                                          gint          len);

static gboolean  udp_prepare             (GSource      *source,
                                          gint         *timeout);
static gboolean  udp_check               (GSource      *source);
static gboolean  udp_dispatch            (GSource      *source,
                                          GSourceFunc   callback,
                                          gpointer      user_data);


typedef struct _BlIsdnSource BlIsdnSource;
struct _BlIsdnSource
{
  GSource  source;
  BlIsdn  *isdn;
};

static GSourceFuncs udp_funcs =
{
  udp_prepare,
  udp_check,
  udp_dispatch,
  NULL
};

enum
{
  INCOMING_CALL,
  STATE_CHANGED,
  KEY_PRESSED,
  LAST_SIGNAL
};
static guint bl_isdn_signals[LAST_SIGNAL] = { 0 };

static GObjectClass *parent_class = NULL;


/* BlIsdn source function */

static gboolean
udp_prepare (GSource *source,
             gint    *timeout)
{
  BlIsdn         *isdn = ((BlIsdnSource *) source)->isdn;
  fd_set          set;
  struct timeval  tv;

  FD_ZERO (&set);
  FD_SET (isdn->listen_sock, &set);
  tv.tv_sec  = 0;
  tv.tv_usec = 0;

  isdn->packet_pending = (select (isdn->listen_sock + 1,
                                  &set, NULL, NULL, &tv) > 0);

  if (timeout)
    *timeout = POLL_TIMEOUT;

  if (isdn->packet_pending)
    isdn->no_packets = 0;
  else
    isdn->no_packets++;

  if (isdn->no_packets == (RESET_TIMEOUT * 1000 / POLL_TIMEOUT))
    {
      g_printerr ("BlIsdn: %d seconds have gone by without any sign of life\n"
                  "        from the ISDN system. I'll try to reconnect ...\n",
                  RESET_TIMEOUT);

      bl_isdn_teardown (isdn);
      bl_isdn_setup (isdn, NULL);
    }

  return isdn->packet_pending;
}

static gboolean
udp_check (GSource *source)
{
  BlIsdn *isdn = ((BlIsdnSource *) source)->isdn;

  return isdn->packet_pending;
}

static gboolean
udp_dispatch (GSource     *source,
              GSourceFunc  callback,
              gpointer     user_data)
{
  BlIsdn         *isdn  = ((BlIsdnSource *) source)->isdn;
  gchar           buffer[2048];
  fd_set          set;
  struct timeval  tv;

  isdn->packet_pending = FALSE;

  FD_ZERO (&set);
  FD_SET (isdn->listen_sock, &set);
  tv.tv_sec  = 0;
  tv.tv_usec = 0;

  /* on first call empty the queue */
  if (!isdn->sock_initialized)
    {
      while (select (isdn->listen_sock + 1, &set, NULL, NULL, &tv) > 0)
        {
          FD_ZERO (&set);
          FD_SET (isdn->listen_sock, &set);
          tv.tv_sec  = 0;
          tv.tv_usec = 0;

          if (FD_ISSET (isdn->listen_sock, &set))
            read (isdn->listen_sock, buffer, sizeof (buffer));
        }
      isdn->sock_initialized = TRUE;
    }

  while (select (isdn->listen_sock + 1, &set, NULL, NULL, &tv) > 0)
    {
      FD_ZERO (&set);
      FD_SET (isdn->listen_sock, &set);
      tv.tv_sec  = 0;
      tv.tv_usec = 0;

      if (FD_ISSET (isdn->listen_sock, &set))
        {
          gint bytes;

          bytes = read (isdn->listen_sock, buffer, sizeof (buffer) - 1);

          if (bytes > 0)
            {
              buffer[bytes] = '\0';

              if (!isdn->blocked)
                bl_isdn_handle_message (isdn, buffer, bytes);
            }
        }
    }

  return TRUE;
}

/* BlIsdn GObject framework */

GType
bl_isdn_get_type (void)
{
  static GType isdn_type = 0;

  if (!isdn_type)
    {
      static const GTypeInfo isdn_info =
      {
        sizeof (BlIsdnClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) bl_isdn_class_init,
	NULL,		/* class_finalize */
	NULL,		/* class_data     */
	sizeof (BlIsdn),
	0,              /* n_preallocs    */
	(GInstanceInitFunc) bl_isdn_init,
      };

      isdn_type = g_type_register_static (G_TYPE_OBJECT,
                                          "BlIsdn",
                                          &isdn_info, 0);
    }

  return isdn_type;
}

static void
bl_isdn_class_init (BlIsdnClass *class)
{
  GObjectClass *object_class;

  parent_class = g_type_class_peek_parent (class);

  object_class = G_OBJECT_CLASS (class);

  bl_isdn_signals[INCOMING_CALL] =
    g_signal_new ("incoming_call",
		  G_TYPE_FROM_CLASS (class),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (BlIsdnClass, incoming_call),
		  NULL, NULL,
		  bl_marshal_VOID__POINTER,
		  G_TYPE_NONE, 1, G_TYPE_POINTER);
  bl_isdn_signals[STATE_CHANGED] =
    g_signal_new ("state_changed",
		  G_TYPE_FROM_CLASS (class),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (BlIsdnClass, state_changed),
		  NULL, NULL,
		  bl_marshal_VOID__POINTER,
		  G_TYPE_NONE, 1, G_TYPE_POINTER);
  bl_isdn_signals[KEY_PRESSED] =
    g_signal_new ("key_pressed",
    		  G_TYPE_FROM_CLASS (class),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (BlIsdnClass, key_pressed),
		  NULL, NULL,
		  bl_marshal_VOID__POINTER_INT,
		  G_TYPE_NONE, 2, G_TYPE_POINTER, G_TYPE_INT);

  class->incoming_call = NULL;
  class->state_changed = NULL;
  class->key_pressed   = NULL;

  object_class->finalize = bl_isdn_finalize;
}

static void
bl_isdn_init (BlIsdn *isdn)
{
  isdn->listen_sock       = -1;
  isdn->send_sock         = -1;
  isdn->heartbeat         = 0;
  isdn->sock_initialized  = FALSE;
  isdn->state_initialized = FALSE;
  isdn->packet_pending    = FALSE;
  isdn->no_packets        = 0;
  isdn->blocked           = FALSE;
  isdn->lines             = NULL;
}

static void
bl_isdn_finalize (GObject *object)
{
  BlIsdn *isdn = BL_ISDN (object);

  bl_isdn_teardown (isdn);

  if (isdn->source)
    {
      g_source_destroy (isdn->source);
      isdn->source = NULL;
    }
  if (isdn->lines)
    {
      g_free (isdn->lines);
      isdn->lines = NULL;
    }
  if (isdn->config)
    {
      g_object_unref (isdn->config);
      isdn->config = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
bl_isdn_setup (BlIsdn  *isdn,
               GError **error)
{
  BlConfig           *config;
  struct sockaddr_in  addr;
  struct hostent     *dest;
  gchar              *cmd;
  gint                port;
  gint                i;

  config = isdn->config;

  dest = gethostbyname (config->isdn_host);
  if (dest == NULL)
    {
      g_set_error (error, 0, 0,
                   "Unable to resolve host '%s'", config->isdn_host);
      return FALSE;
    }

  if ((isdn->listen_sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) != -1)
    {
      memset (&addr, 0, sizeof (addr));

      addr.sin_family      = AF_INET;
      addr.sin_port        = htons (config->isdn_listen);
      addr.sin_addr.s_addr = INADDR_ANY;

      if ((bind (isdn->listen_sock,
                 (struct sockaddr *) &addr, sizeof (addr))) == -1)
        {
          g_set_error (error, 0, 0, "Failed to bind listening socket: %s",
                       g_strerror (errno));
          return FALSE;
        }
    }
  else
    {
      g_set_error (error, 0, 0,
                   "Failed to open listening socket: %s", g_strerror (errno));
      return FALSE;
    }

  port = ntohs (addr.sin_port);

  memset (&addr, 0, sizeof (addr));

  addr.sin_family = AF_INET;
  addr.sin_port   = htons (config->isdn_port);
  memcpy (&addr.sin_addr.s_addr, dest->h_addr_list[0], dest->h_length);

  if ((isdn->send_sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) != -1)
    {
      i = 1;
      if ((setsockopt (isdn->send_sock,
                       SOL_SOCKET, SO_REUSEADDR, &i, sizeof (i))) == -1)
	{
          g_set_error (error, 0, 0, "Failed to configure send socket: %s",
                       g_strerror (errno));
          return FALSE;
        }

      if ((connect (isdn->send_sock,
                    (struct sockaddr *) &addr, sizeof (addr))) == -1)
        {
          g_set_error (error, 0, 0,
                       "Failed to bind send socket: %s", g_strerror (errno));
          return FALSE;
        }
    }
  else
    {
      g_set_error (error, 0, 0,
                   "Failed to open send socket: %s", g_strerror (errno));
      return FALSE;
    }

  cmd = g_strdup_printf ("0:register:%d", port);
#if ISDN_VERBOSE
  g_printerr ("BlIsdn > %s\n", cmd);
#endif
  if (send (isdn->send_sock, cmd, strlen (cmd), 0) == -1)
    {
      g_set_error (error, 0, 0,
                   "Failed to send register message: %s", g_strerror (errno));
      g_free (cmd);
      return FALSE;
    }
  g_free (cmd);

  g_printerr ("Connected to ISDN system (%s:%d), listening on port %d\n",
              config->isdn_host, config->isdn_port, port);

  bl_isdn_heartbeat_start (isdn);

  return TRUE;
}

static void
bl_isdn_teardown (BlIsdn *isdn)
{
  bl_isdn_heartbeat_stop (isdn);

  if (isdn->listen_sock > -1)
    {
      close (isdn->listen_sock);
      isdn->listen_sock      = -1;
      isdn->sock_initialized = FALSE;
    }

  isdn->no_packets = 0;

  bl_isdn_hangup_all (isdn);

  if (isdn->send_sock > -1)
    {
      close (isdn->send_sock);
      isdn->send_sock = -1;
    }
}

static void
bl_isdn_hangup_all (BlIsdn *isdn)
{
  BlIsdnLine *line;
  gint        i;

  for (i = 0, line = isdn->lines; i < isdn->num_lines; i++, line++)
    {
      if (line->offhook)
        {
          gchar *msg = g_strdup_printf ("%d:hangup", i + 1);

          send (isdn->send_sock, msg, strlen (msg), 0);

          line->offhook = FALSE;
          if (!isdn->blocked)
            g_signal_emit (G_OBJECT (isdn),
                           bl_isdn_signals[STATE_CHANGED], 0, line, NULL);

          g_free (line->called_number);
          g_free (line->calling_number);
        }
    }
}

static gboolean
bl_isdn_heartbeat (BlIsdn *isdn)
{
  const gchar *msg = "0:heartbeat";

  send (isdn->send_sock, msg, strlen (msg), 0);

  return TRUE;
}

static void
bl_isdn_heartbeat_start (BlIsdn *isdn)
{
  g_return_if_fail (isdn->heartbeat == 0);

  isdn->heartbeat = g_timeout_add (10000,
                                   (GSourceFunc) bl_isdn_heartbeat, isdn);
}

static void
bl_isdn_heartbeat_stop (BlIsdn *isdn)
{
  if (isdn->heartbeat)
    {
      g_source_remove (isdn->heartbeat);
      isdn->heartbeat = 0;
    }
}

static void
bl_isdn_handle_message (BlIsdn      *isdn,
                        const gchar *msg,
                        gint         len)
{
  gint channel;

  if (len < 2 || msg[1] != ':')
    return;

  channel = msg[0] - '0';
  msg += 2;

  if (channel > 0 && channel <= isdn->num_lines)
    {
      BlIsdnLine *line = isdn->lines + channel - 1;

      g_return_if_fail (line->channel == channel);

      if (line->offhook)
        {
          if (strncmp (msg, "onhook", 6) == 0)
            {
#if ISDN_VERBOSE
              g_printerr ("BlIsdn: Line %d hung up\n", channel);
#endif
              line->offhook = FALSE;
              g_signal_emit (G_OBJECT (isdn),
                             bl_isdn_signals[STATE_CHANGED], 0, line);

              g_free (line->calling_number);
              line->calling_number = NULL;

              g_free (line->called_number);
              line->called_number = NULL;

              return;
            }
          else if (strncmp (msg, "dtmf:", 5) == 0)
            {
              BModuleKey  key;

              switch (msg[5])
                {
                case '0'...'9':
                  key = msg[5] - '0';
                  break;
                case '#':
                  key = B_KEY_HASH;
                  break;
                case '*':
                  key = B_KEY_ASTERISK;
                  break;
                default:
                  g_printerr ("BlIsdn: Unknown DTMF key '%s'\n", msg + 5);
                  return;
                }

#if ISDN_VERBOSE
              g_printerr ("BlIsdn: Key %c pressed on line %d\n",
                          msg[5], channel);
#endif
              g_signal_emit (G_OBJECT (isdn),
                             bl_isdn_signals[KEY_PRESSED], 0, line, key);
              return;
            }
        }
      else /* onhook */
        {
          if (strncmp (msg, "setup:", 6) == 0)
            {
              const gchar *caller = msg + 6;
              gchar       *called;

              if ((called = strchr (caller, ':')))
                *called++ = '\0';

#if ISDN_VERBOSE
              g_printerr ("BlIsdn: incoming call on line %d from %s to %s\n",
                          channel, caller, called);
#endif
              g_free (line->called_number);
              line->called_number  = g_strdup (called);

              g_free (line->calling_number);
              line->calling_number = g_strdup (caller);

              g_signal_emit (G_OBJECT (isdn),
                             bl_isdn_signals[INCOMING_CALL], 0, line);
              return;
            }
          else if (strncmp (msg, "connected", 9) == 0)
            {
#if ISDN_VERBOSE
              g_printerr ("BlIsdn: line %d connected\n", channel);
#endif
              line->offhook = TRUE;
              g_signal_emit (G_OBJECT (isdn),
                             bl_isdn_signals[STATE_CHANGED], 0, line);
              return;
            }
        }
    }
  else if (channel != 0)
    {
      g_printerr ("BlIsdn: line %d is out of range (%d lines configured)\n",
                  channel, isdn->num_lines);
      return;
    }

  if (strncmp (msg, "error:", 6) == 0)
    {
      if (channel == 0)
        g_printerr ("BlIsdn: General error signalled: %s\n", msg + 6);
      else
        g_printerr ("BlIsdn: Error signalled on line %d: %s\n",
                    channel, msg + 6);
    }
}

BlIsdn *
bl_isdn_new (BlConfig  *config,
             GError   **error)
{
  BlIsdn         *isdn;
  struct hostent *dest;
  gint            i;

  g_return_val_if_fail (BL_IS_CONFIG (config), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);
  g_return_val_if_fail (config->isdn_host != NULL, NULL);
  g_return_val_if_fail (config->isdn_port > 0, NULL);
  g_return_val_if_fail (config->isdn_listen > 0, NULL);
  g_return_val_if_fail (config->isdn_lines < 9, NULL);

  dest = gethostbyname (config->isdn_host);
  if (dest == NULL)
    {
      g_set_error (error, 0, 0,
                   "Unable to resolve host '%s'", config->isdn_host);
      return FALSE;
    }

  isdn = BL_ISDN (g_object_new (BL_TYPE_ISDN, NULL));

  isdn->config = g_object_ref (config);

  if (! bl_isdn_setup (isdn, error))
    {
      bl_isdn_teardown (isdn);
      g_object_unref (isdn);
      return NULL;
    }

  isdn->num_lines = config->isdn_lines;
  isdn->lines   = g_new0 (BlIsdnLine, isdn->num_lines);

  for (i = 0; i < isdn->num_lines; i++)
    isdn->lines[i].channel = i + 1;

  isdn->source = g_source_new (&udp_funcs, sizeof (BlIsdnSource));
  g_source_set_priority (isdn->source, G_PRIORITY_DEFAULT);
  g_source_set_can_recurse (isdn->source, FALSE);

  ((BlIsdnSource *) isdn->source)->isdn = isdn;
  g_source_attach (isdn->source, NULL);

  return isdn;
}

void
bl_isdn_call_accept (BlIsdn      *isdn,
                     BlIsdnLine  *line,
                     const gchar *sound,
                     const gchar *sound_loop)
{
  gchar *cmd;

  g_return_if_fail (BL_IS_ISDN (isdn));
  g_return_if_fail (line != NULL);

  if (isdn->blocked)
    return;

  cmd = g_strdup_printf ("%d:accept", line->channel);
#if ISDN_VERBOSE
  g_printerr ("BlIsdn > %s\n", cmd);
#endif
  send (isdn->send_sock, cmd, strlen (cmd), 0);
  g_free (cmd);

  if (sound_loop || sound)
    {
      cmd = g_strdup_printf ("%d:playbackground:%s",
                             line->channel,
                             sound_loop ? sound_loop : sound);
#if ISDN_VERBOSE
  g_printerr ("BlIsdn > %s\n", cmd);
#endif
      send (isdn->send_sock, cmd, strlen (cmd), 0);
      g_free (cmd);
    }

  if (sound_loop && sound)
    {
      cmd = g_strdup_printf ("%d:play:%s", line->channel, sound);
#if ISDN_VERBOSE
  g_printerr ("BlIsdn > %s\n", cmd);
#endif
      send (isdn->send_sock, cmd, strlen (cmd), 0);
      g_free (cmd);
    }
}

void
bl_isdn_call_hangup (BlIsdn       *isdn,
                     BlIsdnLine   *line,
                     BlIsdnReason  reason)
{
  gchar *cmd;

  g_return_if_fail (BL_IS_ISDN (isdn));
  g_return_if_fail (line != NULL);

  if (reason)
    cmd = g_strdup_printf ("%d:hangup:%d", line->channel, reason);
  else
    cmd = g_strdup_printf ("%d:hangup", line->channel);

#if ISDN_VERBOSE
  g_printerr ("BlIsdn > %s\n", cmd);
#endif
  send (isdn->send_sock, cmd, strlen (cmd), 0);
  g_free (cmd);
}

void
bl_isdn_block (BlIsdn *isdn)
{
  g_return_if_fail (BL_IS_ISDN (isdn));

  bl_isdn_hangup_all (isdn);

  isdn->blocked = TRUE;

  g_printerr ("BlIsdn: Thrown out all callers, ISDN is now blocked!\n");
}

void
bl_isdn_unblock (BlIsdn *isdn)
{
  g_return_if_fail (BL_IS_ISDN (isdn));

  isdn->blocked = FALSE;

  g_printerr ("BlIsdn: Unblocked ISDN!\n");
}
