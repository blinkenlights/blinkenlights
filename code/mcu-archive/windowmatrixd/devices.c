/*
   windowmatrixd - receives blinkenframes from the network

   Copyright (C) 2001, 2002 Sebastian Klemke

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/


#include "common.h"


/* local variables */

static const char rcsid[] = "$Id: devices.c,v 1.1 2002/05/31 22:49:36 tim Exp $";


/* setup_devices
This function initializes the output devices. */

device_t *
setup_devices (void)
{
  unsigned char    table[NUMBER_OF_DEVS][8];
  device_t      *  devices;
  int              i;
  char          *  devicename, c;

  memset (&table, 0, sizeof (table));
  for (i = 0; (i < (WIDTH * HEIGHT)); i++)
    {
      if ((((table_t *)remap_table)[i].table_devicenum < 0)      ||           \
	  (((table_t *)remap_table)[i].table_devicenum >                      \
	   (NUMBER_OF_DEVS - 1))                                 ||           \
	  (((table_t *)remap_table)[i].table_bitnum    < 0)      ||           \
	  (((table_t *)remap_table)[i].table_bitnum    > 7)      ||           \
	  (table[((table_t *)remap_table)[i].table_devicenum]                 \
	   [((table_t *)remap_table)[i].table_bitnum] != (unsigned char) 0))
	{
	  log_printf (LOG_ERR, "invalid remap-table, exiting.");
	  return (NULL);
	}
      table[((table_t *)remap_table)[i].table_devicenum]                      \
	[((table_t *)remap_table)[i].table_bitnum] = (unsigned char) 1;
    }

  if ((devices = (device_t *) malloc (sizeof (device_t) * NUMBER_OF_DEVS)))
    {
      for (i = 0; (i < NUMBER_OF_DEVS) && devices; i++)
	{
	  devices[i].dev_fd = -1;
	  devices[i].dev_buffer = (unsigned char) 0;
	}
      for (i = 0; (i < NUMBER_OF_DEVS) && devices; i++)
	{
	  asprintf (&devicename, "/dev/pio%i", i);
	  if ((devices[i].dev_fd = open (devicename, O_RDWR)) != -1)
	    {
	      c = (char) BMC_PIO_OUTPUT;
	      if (!(ioctl (devices[i].dev_fd, BMC_PIO_CTL_DIRECTION, &c)))
		{
		  c = (char) BMC_PIO_GROUPA_M0;
		  if (!(ioctl (devices[i].dev_fd, BMC_PIO_CTL_SETMODE_A, &c)))
		    {
		      c = (char) BMC_PIO_GROUPB_M0;
		      if (!(ioctl (devices[i].dev_fd, BMC_PIO_CTL_SETMODE_B,  \
				   &c)))
			continue;
		    }
		}
	    }
	  free (devicename);
	  close_devices (devices);
	  devices = NULL;
	  break;
	}
    }
  return (devices);
}


/* close_devices
This function closes the output devices. */

void
close_devices (device_t *  devices)
{
  int i;
  for (i = 0; (i < NUMBER_OF_DEVS) && (devices[i].dev_fd != -1); i++)
    {
      close (devices[i].dev_fd);
    }
  free (devices);
}


/* output_frame
This function "draws" the frame on the Windows. */

inline void
output_frame (const bl_frame_t *  frame, device_t *  devices)
{
  int           x, y;

  printf("\033[2J\033[H");
  for (y = 0; y < HEIGHT; y++)
    {
      for (x = 0; x < WIDTH; x++)
	{
	  if (frame->frame_data[y][x])
	    {
	      devices[remap_table[y][x].table_devicenum].dev_buffer +=        \
		(1 << (remap_table[y][x].table_bitnum));
	      interactive_printf ("#");
	    }
	  else
	    {
	      interactive_printf (" ");
	    }
	}
      interactive_printf("\n");
    }
  for (x = 0; (x < NUMBER_OF_DEVS) && devices; x++)
    {
      write (devices[x].dev_fd, &(devices[x].dev_buffer), 1);
      devices[x].dev_buffer = (unsigned char) 0;
    }
}
