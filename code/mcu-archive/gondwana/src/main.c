/* gonwdwana - a simple bushfire simulator
 * Bushfire is a Blinkenlights Installation (TM)
 *
 * Copyright (c) 2002  Sven Neumann <sven@gimp.org>
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
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <directfb.h>

#include "magic-values.h"
#include "gfx.h"
#include "network.h"
#include "effects.h"


int
main (int     argc, 
      char ** argv)
{
  IDirectFB            *dfb;
  IDirectFBEventBuffer *keys;
  DFBResult             dfb_result;
  struct sockaddr_in    addr;
  int                   sock;

  int                   effect_invert  = FALSE;
  int                   effect_hflip   = FALSE;
  int                   effect_vflip   = FALSE;
  int                   effect_lmirror = FALSE;
  int                   effect_rmirror = FALSE;

  if ((dfb_result = DirectFBInit (&argc, &argv)) != DFB_OK)
    {
      fprintf (stderr, "Initialization of DirectFB failed: %s\n", 
               DirectFBErrorString (dfb_result));
      return EXIT_FAILURE;
    }

  if (argc > 1 && strcmp (argv[1], "--test") == 0)
    {
      dfb = setup_gfx ();
      if (!dfb)
        return EXIT_FAILURE;

      dfb->CreateEventBuffer (dfb, DICAPS_KEYS, &keys);

      output_frame (get_test_frame ());

      keys->WaitForEvent (keys);
      keys->Release (keys);
      close_gfx ();

      return EXIT_SUCCESS;
    }

  sock = setup_socket (&addr);
  
  if (sock < 0)
    {
      perror ("Failed to open socket");
      return EXIT_FAILURE;
    }

  {
    const unsigned char  *frame = NULL;
    struct timeval        tv;
    fd_set                set;
    int                   result;
    
    dfb = setup_gfx ();
    if (!dfb)
      {
        close (sock);
        return EXIT_FAILURE;
      }

    dfb->CreateEventBuffer (dfb, DICAPS_KEYS, &keys);
    
    FD_ZERO (&set);
    FD_SET (sock, &set);
    tv.tv_sec  = 0;
    tv.tv_usec = KEYBOARD_TIMEOUT;
    
    while ((result = select (sock+1, &set, NULL, NULL, &tv)) >= 0 || 
           errno == EINTR)
      {
        if (result > 0)  /*  socket is readable  */
          {
            frame = get_frame (sock, &addr);
            if (frame)
              {
                apply_effects ((unsigned char *) frame,
                               effect_invert,
                               effect_vflip,
                               effect_hflip,
                               effect_lmirror,
                               effect_rmirror);

                output_frame (frame);
              }
          }
        
        if (keys->HasEvent (keys) == DFB_OK)
          {
            DFBInputEvent event;

            if (keys->GetEvent (keys, DFB_EVENT (&event)) == DFB_OK)
              {
                if ((event.type == DIET_KEYPRESS) &&
                    (event.flags & DIEF_KEYSYMBOL))
                  {
                    DFBInputDeviceKeySymbol  key_symbol;

                    key_symbol = DFB_LOWER_CASE (event.key_symbol);

                    switch (key_symbol)
                      {
                      case DIKS_SMALL_Q:
                      case DIKS_ESCAPE:
                        goto finish;

                      case DIKS_SMALL_I:
                        effect_invert = ! effect_invert;
                        break;

                      case DIKS_SMALL_H:
                        effect_hflip = ! effect_hflip;
                        break;

                      case DIKS_SMALL_V:
                        effect_vflip = ! effect_vflip;
                        break;

                      case DIKS_SMALL_L:
                        effect_lmirror = ! effect_lmirror;
                        if (effect_lmirror)
                          effect_rmirror = FALSE;
                        break;

                      case DIKS_SMALL_R:
                        effect_rmirror = ! effect_rmirror;
                        if (effect_rmirror)
                          effect_lmirror = FALSE;
                        break;

                      default:
                        break;
                      }
                  }
              }
          }
        
        FD_ZERO (&set);
        FD_SET (sock, &set);
        tv.tv_sec  = 0;
        tv.tv_usec = KEYBOARD_TIMEOUT;
      }
  }
  
 finish:

  close (sock);
    
  keys->Release (keys);
  close_gfx ();

  return EXIT_SUCCESS;
}
