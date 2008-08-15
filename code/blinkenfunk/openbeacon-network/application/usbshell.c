/* usbshell.c - command line interface for configuration and status inquiry
 *
 * Copyright (c) 2008  The Blinkenlights Crew
 *                          Daniel Mack <daniel@caiaq.de>
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


/* Standard includes. */
#include <stdio.h>
#include <string.h>

/* Scheduler includes. */
#include <FreeRTOS.h>
#include <board.h>
#include <task.h>
#include <semphr.h>
#include <led.h>

#include "debug_printf.h"
#include "network.h"
#include "SAM7_EMAC.h"
#include "env.h"
#include "USB-CDC.h"

#define PROMPT "\n\rWMCU> "

#define shell_printf(x...) debug_printf(x)

static void
cmd_status (const portCHAR * cmd)
{
  shell_printf ("alled schnafte\n");
}

static void
cmd_mac (const portCHAR * cmd)
{
  portCHAR buf[4], mac_l, mac_h;
  unsigned int i;

  while (*cmd && *cmd != ' ')
  	cmd++;

  cmd++;

  for (i = 0; i < sizeof(buf); i++) {
    if (!*cmd) {
      shell_printf("bogus command.\n");
      return;
    }

    buf[i] = *cmd++;
    if (buf[i] >= 'A' && buf[i] <= 'F')
      buf[i] -= 'A';
    else if (buf[i] >= 'a' && buf[i] <= 'f')
      buf[i] -= 'a';
    else if (buf[i] >= '0' && buf[i] <= '9')
      buf[i] -= '0';
    else
      {
        shell_printf("invalid MAC!\n");
        return;
      }
  }

  mac_h = buf[0] << 4 | buf[1];
  mac_l = buf[2] << 4 | buf[3];

  /* checksum given? */
  if (*cmd == ' ')
    {
       portCHAR crc;

       buf[0] = *cmd++;
       if (!*cmd)
         {
	   shell_printf("bogus checksum!\n");
	   return;
	 }
       
       buf[1] = *cmd++;
       crc = buf[0] << 4 | buf[1];

       if (crc != (mac_l ^ mac_h))
         {
	   shell_printf("invalid checksum - command ignored\n");
	   return;
	 }
    }
   
    shell_printf("parsed MAC: %02x%02x\n", mac_h, mac_l);

    /* set it ... */
    env.e.mac_h = mac_h;
    env.e.mac_l = mac_l;
}

static void
cmd_env (const portCHAR * cmd)
{
  shell_printf ("Current values in non-volatile flash storage:\n");
  shell_printf ("   assigned_line = %d\n", env.e.assigned_line);
  shell_printf ("   mac = %02x%02x\n", env.e.mac_h, env.e.mac_l);
}

static void
parse_cmd (const portCHAR * cmd)
{
  if (strlen (cmd) == 0)
    return;

  if (strcmp (cmd, "status") == 0)
    cmd_status (cmd);
  else if (strcmp (cmd, "env") == 0)
    cmd_env (cmd);
  else if (strncmp (cmd, "mac", strlen("mac")) == 0 || 
           strncmp (cmd, "wmcu-mac", strlen("wmc-mac")) == 0)
    cmd_mac (cmd);
  else
    shell_printf ("unknown command '%s'\n", cmd);
}

static void
usbshell_task (void *pvParameters)
{
  int size = 0;
  static portCHAR buf[128], c, *p = buf;
  bool_t overflow = pdFALSE;
  
  for (;;)
    {
      if (vUSBRecvByte (&c, sizeof (c), 100))
	{
	  vUSBSendByte (c);

	  if (c < ' ')
	    {
	      *p = '\0';
	      if (overflow)
		overflow = pdFALSE;
	      else
		parse_cmd (buf);

	      p = buf;
	      size = 0;
	      shell_printf (PROMPT);
	    }
	  else if (size < ((int) sizeof (buf) - 1))
	    {
	      *p++ = c;
	      size++;
	    }
	  else
	    overflow = pdTRUE;
	}
    }
}

void
vUSBShellInit (void)
{
  xTaskCreate (usbshell_task, (signed portCHAR *) "USBSHELL",
	       TASK_USBSHELL_STACK, NULL, TASK_USBSHELL_PRIORITY, NULL);
}
