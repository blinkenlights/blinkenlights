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
#include <beacontypes.h>
#include "debug_print.h"
#include "env.h"
#include "USB-CDC.h"
#include "dimmer.h"

#define PROMPT "\nWDIM> "

#define shell_print(x) DumpStringToUSB(x)

static void
cmd_status (const portCHAR * cmd)
{
}

static void
cmd_help (const portCHAR *cmd)
{
  shell_print("Blinkenlights command shell help.\n");
  shell_print("---------------------------------\n");
  shell_print("\n");
  shell_print("help\n");
  shell_print("	This screen\n");
  shell_print("\n");
  shell_print("[wdim-]mac <xxyy> [<crc>]\n");
  shell_print("	Set the MAC address of this unit.\n");
  shell_print("\n");
  shell_print("status\n");
  shell_print("	Print status information about this unit. Try it, it's fun.\n");
  shell_print("\n");
  shell_print("env\n");
  shell_print("	Show variables currently stored in the non-volatile flash memory\n");
  shell_print("\n");
  shell_print("dim <value>\n");
  shell_print("	Set the dimmer to a value (between 0 and 10000)\n");
  shell_print("");
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
      shell_print("bogus command.\n");
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
        shell_print("invalid MAC!\n");
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
	   shell_print("bogus checksum!\n");
	   return;
	 }
       
       buf[1] = *cmd++;
       crc = buf[0] << 4 | buf[1];

       if (crc != (mac_l ^ mac_h))
         {
	   shell_print("invalid checksum - command ignored\n");
	   return;
	 }
    }
   
    shell_print("setting new MAC.\n");
    shell_print("Please power-cycle the device to make this change take place.\n");

    /* set it ... */

/*
    env.e.mac_h = mac_h;
    env.e.mac_l = mac_l;
    env_store();
*/
}

static void
cmd_env (const portCHAR * cmd)
{
  shell_print ("Current values in non-volatile flash storage:\n");
//  shell_print ("   assigned_line = %d\n", env.e.assigned_line);
//  shell_print ("   mac = %02x%02x\n", env.e.mac_h, env.e.mac_l);
}

static void
cmd_dim (const portCHAR * cmd)
{
  int val = 0;

  while (*cmd && *cmd != ' ')
  	cmd++;

  cmd++;

  while (*cmd >= '0' && *cmd <= '9')
    {
      val *= 10;
      val += *cmd - '0';
      cmd++;
    }

  vUpdateDimmer(val);
  shell_print("setting dimmer to value ");
  DumpUIntToUSB(val);
  shell_print("\n");
}


static struct cmd_t {
	const portCHAR *command;
	void (*callback) (const portCHAR *cmd);
} commands[] = {
	{ "help",	&cmd_help },
	{ "status",	&cmd_status },
	{ "mac",	&cmd_mac },
	{ "wdim-mac",	&cmd_mac },
	{ "env",	&cmd_env },
	{ "dim",	&cmd_dim },
	/* end marker */
	{ NULL, NULL }
};

static void
parse_cmd (const portCHAR * cmd)
{
  struct cmd_t *c;

  if (strlen (cmd) == 0)
    return;

  for (c = commands; c && c->command && c->callback; c++)
    {
      if (strncmp (cmd, c->command, strlen(c->command)) == 0 && c->callback)
        {
          c->callback(cmd);
	  return;
	}
    }

  shell_print("unknown command '");
  shell_print(cmd);
  shell_print("'\n");
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
	      shell_print (PROMPT);
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
