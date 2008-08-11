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
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <blib/blib.h>

#include "bltypes.h"

#include "blaccept.h"
#include "blccc.h"
#include "bllisten.h"


const gchar *welcome_msg =
"--------------------------------------------------\n"
"Welcome to the Blinkenlights Chaos Control Center.\n"
"--------------------------------------------------\n"
"Type 'help' to get a list of supported commands.\n";

const gchar *help_msg =
"Supported commands are:\n"
"  bye              - closes the connection\n"
"  help             - shows this list of supported commands\n"
"  isdn block       - block control through isdn lines\n"
"  isdn unblock     - allow control through isdn lines\n"
"  enable <number>  - enable application\n"
"  disable <number> - disable application\n"
"  list             - list items in current playlist\n"
"  load <filename>  - load new playlist\n"
"  start <filename> - load new playlist and start immidiately\n"
"  next             - skip to next item\n"
"  reload           - reload current playlist\n"
"  status           - show status info\n"
"  add <host>       - add recipient\n"
"  remove <host>    - remove recipient\n"
#if 0
"  love <id>        - play loveletter <id>\n"
#endif
"  i                - invert display\n"
"  v                - flip display vertically\n"
"  h                - flip display horizontally\n"
"  l                - mirror left side\n"
"  r                - mirror right side\n"
"  0..9 # *         - fake isdn event\n";

const gchar *load_msg    = "Playlist loaded.\n";
const gchar *reload_msg  = "Playlist reloaded.\n";
const gchar *added_msg   = "Host added.\n";
const gchar *removed_msg = "Host removed.\n";
const gchar *love_msg    = "Made love.\n";
const gchar *event_msg   = "Event dispatched.\n";
const gchar *ok_msg      = "OK\n";
const gchar *failed_msg  = "Operation failed.\n";
const gchar *timeout_msg = "\nConnection timed out.\n";

const gchar *unknown_msg =
"Unknown command, type 'help' to get a list of supported commands.\n";


typedef enum
{
  READ     = 0,
  WRITE    = 1,
  CONTINUE = 2,
  END      = READ
} BlAcceptState;


typedef struct _BlAccept BlAccept;
struct _BlAccept
{
  gint            sock;
  BlAcceptState   state;
  BlCcc          *ccc;
  GString        *buffer;
};


static void  bl_accept_write (BlAccept    *acc,
                              const gchar *buf,
                              gint         len);
static void  bl_accept_read  (BlAccept    *acc,
                              const gchar *buf,
                              gint         len);

static inline void
bl_accept_write_prompt (BlAccept *acc)
{
  bl_accept_write (acc, "> ", 2);
}


void
bl_accept_new (BlListen *listen)
{
  BlAccept         acc;
  gint             len;
  gchar            buf[1024];
  fd_set           set;
  struct timeval   tv;
  struct sigaction sa;

  g_return_if_fail (BL_IS_LISTEN (listen));

  acc.state = CONTINUE;
  acc.sock  = accept (listen->sock, NULL, 0);

  g_thread_create ((GThreadFunc) bl_accept_new, listen, FALSE, NULL);

  if (acc.sock < 0)
    return;

  /* ignore SIGPIPE for this thread */
  sigfillset (&sa.sa_mask);
  sa.sa_handler = SIG_IGN;
  sa.sa_flags   = SA_RESTART;
  sigaction (SIGPIPE, &sa, NULL);

  acc.buffer = g_string_new (NULL);
  acc.ccc    = g_object_ref (listen->ccc);

  bl_accept_write (&acc, welcome_msg, -1);
  bl_accept_write_prompt (&acc);

  while (acc.state)
    {
      if (acc.state & WRITE)
        {
          if (acc.buffer->len > 0)
            {
              /* wait with a timeout of 2 minutes */
              FD_ZERO (&set);
              FD_SET (acc.sock, &set);
              tv.tv_sec  = 120;
              tv.tv_usec = 0;

              len = select (acc.sock + 1, NULL, &set, NULL, &tv);

              if (len < 0)             /* select() failed             */
                {
                  if (errno != EINTR)  /* and was not only interupted */
                    acc.state = END;

                  continue;
                }
              if (len == 0)            /* select() timed out          */
                {
                  acc.state = END;
                  continue;
                }

              len = write (acc.sock, acc.buffer->str, acc.buffer->len);
              if (len < 0)
                acc.state = END;
              else
                g_string_erase (acc.buffer, 0, len);
            }

          if (acc.buffer->len == 0)
            acc.state &= ~WRITE;

          continue;
        }
      else
        {
          /* wait for input with a timeout of 10 minutes */
          FD_ZERO (&set);
          FD_SET (acc.sock, &set);
          tv.tv_sec  = 600;
          tv.tv_usec = 0;

          len = select (acc.sock + 1, &set, NULL, NULL, &tv);

          if (len < 0)             /* select() failed             */
            {
              if (errno != EINTR)  /* and was not only interupted */
                acc.state = END;

              continue;
            }
          if (len == 0)            /* select() timed out          */
            {
              acc.state = END;
              bl_accept_write (&acc, timeout_msg, -1);
              continue;
            }

          len = read (acc.sock, &buf, sizeof (buf));

          if (len < 0)
            {
              acc.state = END;
              continue;
            }

          bl_accept_read (&acc, buf, len);

          if (acc.state & CONTINUE)
            {
              bl_accept_write_prompt (&acc);
              continue;
            }
        }
    }

  close (acc.sock);

  g_string_free (acc.buffer, TRUE);
  g_object_unref (acc.ccc);
}


static void
bl_accept_write (BlAccept    *acc,
                 const gchar *buf,
                 gint         len)
{
  if (! buf)
    return;

  if (len < 0)
    len = strlen (buf);

  g_string_append_len (acc->buffer, buf, len);

  acc->state |= WRITE;
}

static void
bl_accept_read (BlAccept    *acc,
                const gchar *buf,
                gint         len)
{
  if (len < 0)
    len = strlen (buf);

  acc->state = CONTINUE;

  if ((len > 2 && strncmp (buf, "bye", 3) == 0) ||
      (len > 3 && strncmp (buf, "quit", 4) == 0))
    {
      acc->state = END;
      bl_accept_write (acc, "Good Bye.\n", -1);
      return;
    }
  else if (len > 5 && strncmp (buf, "status", 6) == 0)
    {
      if (acc->ccc)
        {
          gchar *status = bl_ccc_status (acc->ccc);
          bl_accept_write (acc, status, -1);
          g_free (status);
        }
    }
  else if (len > 4 && strncmp (buf, "add ", 4) == 0)
    {
      gchar *host = g_strndup (buf + 4, len - 4);
      g_strstrip (host);
      if (acc->ccc && bl_ccc_add (acc->ccc, host, NULL))
        bl_accept_write (acc, added_msg, -1);
      else
        bl_accept_write (acc, failed_msg, -1);
      g_free (host);
    }
  else if (len > 7 && strncmp (buf, "remove ", 7) == 0)
    {
      gchar *host = g_strndup (buf + 7, len - 7);
      g_strstrip (host);
      if (acc->ccc && bl_ccc_remove (acc->ccc, host, NULL))
        bl_accept_write (acc, removed_msg, -1);
      else
        bl_accept_write (acc, failed_msg, -1);
      g_free (host);
    }
#if 0
  else if (len > 5 && strncmp (buf, "love ", 5) == 0)
    {
      gchar *id = g_strndup (buf + 5, len - 5);
      g_strstrip (id);
      if (acc->ccc && bl_ccc_on_demand (acc->ccc, id))
        bl_accept_write (acc, love_msg, -1);
      else
        bl_accept_write (acc, failed_msg, -1);
      g_free (id);
    }
#endif
  else if (len > 5 && strncmp (buf, "reload", 6) == 0)
    {
      if (acc->ccc && bl_ccc_reload (acc->ccc))
        bl_accept_write (acc, reload_msg, -1);
      else
        bl_accept_write (acc, failed_msg, -1);
    }
  else if (len > 3 && strncmp (buf, "list", 4) == 0)
    {
      if (acc->ccc)
        {
          gchar *list = bl_ccc_list (acc->ccc);
          bl_accept_write (acc, list, -1);
          g_free (list);
        }
    }
  else if (len > 5 && strncmp (buf, "load ", 5) == 0)
    {
      gchar *name = g_strndup (buf + 5, len - 5);
      g_strstrip (name);
      if (acc->ccc && bl_ccc_load (acc->ccc, name, FALSE))
        bl_accept_write (acc, load_msg, -1);
      else
        bl_accept_write (acc, failed_msg, -1);
      g_free (name);
    }
  else if (len > 6 && strncmp (buf, "start ", 6) == 0)
    {
      gchar *name = g_strndup (buf + 6, len - 6);
      g_strstrip (name);
      if (acc->ccc && bl_ccc_load (acc->ccc, name, TRUE))
        bl_accept_write (acc, load_msg, -1);
      else
        bl_accept_write (acc, failed_msg, -1);
      g_free (name);
    }
  else if (len > 3 && strncmp (buf, "next", 4) == 0)
    {
      if (acc->ccc)
        {
          gchar *name = bl_ccc_next (acc->ccc);
          if (name)
            {
              bl_accept_write (acc, "skipped to next item ", -1);
              bl_accept_write (acc, name, -1);
            }
          else
            bl_accept_write (acc, failed_msg, -1);
          g_free (name);
        }
    }
  else if (len > 9 && strncmp (buf, "isdn block", 10) == 0)
    {
      if (acc->ccc)
        bl_ccc_isdn_block (acc->ccc);
    }
  else if (len > 11 && strncmp (buf, "isdn unblock", 12) == 0)
    {
      if (acc->ccc)
        bl_ccc_isdn_unblock (acc->ccc);
    }
  else if (len > 6 && strncmp (buf, "enable ", 7) == 0)
    {
      gchar *num = g_strndup (buf + 7, len - 7);
      g_strstrip (num);
      if (acc->ccc && bl_ccc_app_enable (acc->ccc, num))
        bl_accept_write (acc, ok_msg, -1);
      else
        bl_accept_write (acc, failed_msg, -1);
      g_free (num);
    }
  else if (len > 7 && strncmp (buf, "disable ", 8) == 0)
    {
      gchar *num = g_strndup (buf + 8, len - 8);
      g_strstrip (num);
      if (acc->ccc && bl_ccc_app_disable (acc->ccc, num))
        bl_accept_write (acc, ok_msg, -1);
      else
        bl_accept_write (acc, failed_msg, -1);
      g_free (num);
    }
  else if (len > 3 && strncmp (buf, "help", 4) == 0)
    {
      bl_accept_write (acc, help_msg, -1);
    }
  else if (len > 0 && strncmp (buf, "i", 1) == 0)
    {
      if (acc->ccc && acc->ccc->effects)
        acc->ccc->effects->invert ^= B_EFFECT_SCOPE_ALL;
    }
  else if (len > 0 && strncmp (buf, "v", 1) == 0)
    {
      if (acc->ccc && acc->ccc->effects)
        acc->ccc->effects->vflip ^= B_EFFECT_SCOPE_ALL;
    }
  else if (len > 0 && strncmp (buf, "h", 1) == 0)
    {
      if (acc->ccc && acc->ccc->effects)
        acc->ccc->effects->hflip ^= B_EFFECT_SCOPE_ALL;
    }
  else if (len > 0 && strncmp (buf, "l", 1) == 0)
    {
      if (acc->ccc && acc->ccc->effects)
        {
          acc->ccc->effects->lmirror ^= B_EFFECT_SCOPE_ALL;

          if (acc->ccc->effects->lmirror)
            acc->ccc->effects->rmirror = B_EFFECT_SCOPE_NONE;
        }
    }
  else if (len > 0 && strncmp (buf, "r", 1) == 0)
    {
      if (acc->ccc && acc->ccc->effects)
        {
          acc->ccc->effects->rmirror ^= B_EFFECT_SCOPE_ALL;

          if (acc->ccc->effects->rmirror)
            acc->ccc->effects->lmirror = B_EFFECT_SCOPE_NONE;
        }
    }
  else if (len > 1 &&
           ((*buf >= '0' && *buf <= '9') || *buf == '#' || *buf == '*'))
    {
      if (acc->ccc)
        {
          BModuleEvent event;

          event.device_id = 0;
          event.type      = B_EVENT_TYPE_KEY;

          switch (*buf)
            {
            case '#':
              event.key = B_KEY_HASH;
              break;

            case '*':
              event.key = B_KEY_ASTERISK;
              break;

            default:
              event.key = *buf - '0' + B_KEY_0;
            }

          bl_ccc_event (acc->ccc, &event);
          bl_accept_write (acc, event_msg, -1);
        }
      else
        {
          bl_accept_write (acc, failed_msg, -1);
        }
    }
  else
    {
      bl_accept_write (acc, unknown_msg, -1);
    }
}
