/* usb.c - logging via USB CDC
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
#include <stdarg.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "USB-CDC.h"

void debug_printf(char *fmt, ...)
{
	__VALIST ap;
	char tmp[100], *x;

	memset(tmp, 0, sizeof(tmp));
	va_start(ap, fmt);
	vsnprintf(tmp, sizeof(tmp) - 1, fmt, ap);
	va_end(ap);

	x = tmp;
	while (x && *x) {
		vUSBSendByte(*x);
		if (*x == '\n')
			vUSBSendByte('\r');
		x++;
	}
}


