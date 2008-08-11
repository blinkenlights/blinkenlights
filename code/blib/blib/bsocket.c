/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2004  The Blinkenlights Crew
 *                          Sven Neumann <sven@gimp.org>
 *	                    Daniel Mack <daniel@yoobay.net>
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
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <glib-object.h>

#ifdef G_OS_WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include "bsocket.h"


/**
 * b_socket_init:
 * @error: return location for possible errors.
 *
 * Initialize network sockets. This function does nothing on
 * UNIXes. On Win32 it initializes the Winsock DLL. You may call this
 * function any number of times, the initialization is only done once.
 *
 * Return value: %TRUE on success, %FALSE in case of an error
 **/
gboolean
b_socket_init (GError **error)
{
#ifdef G_OS_WIN32
  static gboolean winsock_initialized = FALSE;

  if (! winsock_initialized)
    {
      WORD    wVersionRequested;
      WSADATA wsaData;

      wVersionRequested = MAKEWORD (2, 2);

      if (WSAStartup (wVersionRequested, &wsaData) == 0)
        {
          winsock_initialized = TRUE;
        }
      else
        {
          g_set_error (error, 0, 0, "Can't initialize the Winsock DLL");
          return FALSE;
        }
    }

#endif
  return TRUE;
}

/**
 * b_socket_udp_new:
 * @options: #BSocketOptions to set on the socket.
 * @error: return location for possible errors.
 *
 * Creates an UDP socket and configures it according to @options.
 *
 * Return value: a socket descriptor or -1 in case of an error
 **/
gint
b_socket_udp_new (BSocketOptions   options,
                  GError         **error)
{
  gint fd;
  gint value;

  g_return_val_if_fail (error == NULL || *error == NULL, -1);

  if (! b_socket_init (error))
    return FALSE;

  fd = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (fd == -1)
    {
      g_set_error (error, 0, 0,
                   "Can't create socket: %s", g_strerror (errno));
      return -1;
    }

  if (options & B_SO_BROADCAST)
    {
      value = 1;
      if (setsockopt (fd,
                      SOL_SOCKET, SO_BROADCAST, &value, sizeof (value)) < 0)
        {
          g_set_error (error, 0, 0,
                       "Can't set socket option (SO_BROADCAST): %s",
                       g_strerror (errno));
          close (fd);
          return -1;
        }
   }

  if (options & B_SO_REUSEADDR)
    {
      value = 1;
      if (setsockopt (fd,
                      SOL_SOCKET, SO_REUSEADDR, &value, sizeof (value)) < 0)
        {
          g_set_error (error, 0, 0,
                       "Can't set socket option (SO_REUSEADDR): %s",
                       g_strerror (errno));
          close (fd);
          return -1;
        }

#ifdef SO_REUSEPORT
      value = 1;
      if (setsockopt (fd,
                      SOL_SOCKET, SO_REUSEPORT, &value, sizeof (value)) < 0)
        {
          g_set_error (error, 0, 0,
                       "Can't set socket option (SO_REUSEPORT): %s",
                       g_strerror (errno));
          close (fd);
          return -1;
        }
#endif
    }

  return fd;
}
