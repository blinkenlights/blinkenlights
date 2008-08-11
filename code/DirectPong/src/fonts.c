/* DirectPong
 *
 * Copyright (c) 2001  Sven Neumann <sven@gimp.org>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fonts.h"


Font *
font_load (const char *filename)
{
  Font          *font = NULL;
  FILE          *fp = NULL;
  char           buf[1024];
  int            i;
  unsigned char *dest;

  fp = fopen (filename, "rb");
  if (!fp)
    goto error;

  fgets (buf, sizeof (buf), fp);
  if (strcmp (buf, "P5\n") != 0)
    goto error;

  font = (Font *) calloc (1, sizeof (Font));

  fgets (buf, sizeof (buf), fp);
  while (buf[0] == '#')
    {
      if (!font->chars && 
          strlen (buf) > 14 && strncmp (buf, "# Characters: ", 14) == 0)
        {
          font->chars = strdup (buf + 14);
          dest = strchr (font->chars, '\n');
          if (dest)
            *dest = '\0';
        }
      else if (strlen (buf) > 11 && strncmp (buf, "# Spacing: ", 11) == 0)
        {
          sscanf (buf + 11, "%d", &font->spacing);
        }
      fgets (buf, sizeof (buf), fp);
    }

  if (!font->chars || strlen (font->chars) < 1)
    goto error;

  if (sscanf (buf, "%d %d", &font->pitch, &font->height) != 2)
    goto error;

  font->width = font->pitch / strlen (font->chars);

  fgets (buf, sizeof (buf), fp);
  if (strcmp (buf, "255\n"))
    goto error;

  font->data = malloc (font->pitch * font->height);
  dest = font->data;
  
  for (i = 0; i < font->height; i++, dest += font->pitch)
    {
      if (fread (dest, sizeof (unsigned char), font->pitch, fp) != font->pitch)
        goto error;
    }

  fclose (fp);
  return font;

 error:
  if (fp)
    fclose (fp);
  if (font)
    {
      if (font->chars)
        free (font->chars);
      if (font->data)
        free (font->data);
      free (font);
    }
  fprintf (stderr, "Couldn't load font from file '%s'.\n", filename);

  return NULL;
}
