/* blccc - BlinkenLigths Chaos Control Center
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
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <gtk/gtk.h>

#include "blccc.h"
#include "bltypes.h"
#include "blmovie.h"


static void      bl_movie_class_init    (BlMovieClass *class);
static void      bl_movie_init          (BlMovie      *movie);
static void      bl_movie_destroy       (GtkObject    *object);
static void      bl_movie_destroy_data  (BlMovie      *movie);
static void      bl_movie_nullify_data  (BlMovie      *movie);
static gboolean  bl_movie_check_file    (const gchar  *filename,
                                         time_t       *mtime);
static gboolean  bl_movie_load_info     (BlMovie      *movie);
static gboolean  bl_movie_load_all      (BlMovie      *movie);

static gboolean  bl_movie_parse_header_line (BlMovie     *movie,
                                             const gchar *buf,
                                             gboolean    *magic);
static void      bl_movie_prepend_frame (BlMovie      *movie, 
                                         gint          duration, 
                                         const guchar *data);


static GtkObjectClass *parent_class = NULL;


GtkType
bl_movie_get_type (void)
{
  static GtkType movie_type = 0;

  if (!movie_type)
    {
      GtkTypeInfo movie_info =
      {
	"BlMovie",
	sizeof (BlMovie),
	sizeof (BlMovieClass),
	(GtkClassInitFunc) bl_movie_class_init,
	(GtkObjectInitFunc) bl_movie_init,
	/* reserved_1 */ NULL,
	/* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL
      };

      movie_type = gtk_type_unique (gtk_object_get_type (), &movie_info);
    }
  
  return movie_type;
}

static void
bl_movie_class_init (BlMovieClass *class)
{
  GtkObjectClass *object_class;

  parent_class = gtk_type_class (gtk_object_get_type ());
  object_class = GTK_OBJECT_CLASS (class);

  object_class->destroy = bl_movie_destroy;
}

static void
bl_movie_init (BlMovie *movie)
{
  movie->filename = NULL;

  bl_movie_nullify_data (movie);
}

static void
bl_movie_destroy (GtkObject *object)
{
  bl_movie_destroy_data (BL_MOVIE (object));

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
bl_movie_destroy_data (BlMovie *movie)
{
  BlMovieFrame *frame;
  GList        *list;

  g_return_if_fail (BL_IS_MOVIE (movie));

  g_free (movie->name);
  g_free (movie->description);
  
  for (list = movie->frames; list; list = g_list_next (list))
    {
      frame = (BlMovieFrame *) list->data;
      g_free (frame->data);
      g_free (frame);
    }
  g_list_free (movie->frames);

  bl_movie_nullify_data (movie);
}

static void
bl_movie_nullify_data (BlMovie *movie)
{
  /* dont nullify filename !! */
  movie->name          = NULL;
  movie->description   = NULL;
  movie->duration      = 0;
  movie->mtime         = 0;
  movie->loaded        = FALSE;
  movie->width         = WIDTH;
  movie->height        = HEIGHT;
  movie->n_frames      = 0;
  movie->frames        = NULL;
}

static gboolean
bl_movie_check_file (const gchar *filename,
                     time_t      *mtime)
{
  struct stat stat_buf;

  g_return_val_if_fail (filename != NULL, FALSE);

  if (stat (filename, &stat_buf))
    return FALSE;

  if (S_ISREG (stat_buf.st_mode) && 
      access (filename, R_OK) == 0)
    {
      if (mtime)
        *mtime = stat_buf.st_mtime;
      
      return TRUE;
    }
  else
    return FALSE;
}

/* die Macintosh die */
static inline char *
fget_line (char *s, int size, FILE *stream)
{
  int i = 0;
  int c = 0;

  if (!s || size < 2)
    return NULL;

  while (i < size - 1)
    {
      c = fgetc (stream);
      if (c == EOF || c == '\r')
        break;
      s[i++] = (char) c;
      if (c == '\n')
        break;
    }

  if (c == '\r')
    {
      c = fgetc (stream);
      if (c != '\n' && c != EOF)
        ungetc (c, stream);
      s[i++] = '\n';
    }
 
  if (i)
    s[i++] = '\0';

  return i > 0 ? s : NULL;
}

static gboolean
bl_movie_load_info (BlMovie *movie)
{
  FILE     *file;
  gchar     buf[1024];
  gboolean  magic = FALSE;

  file = fopen (movie->filename, "r");
  if (!file)
    return FALSE;

  while (fget_line (buf, sizeof (buf), file))
    {
      if (!bl_movie_parse_header_line (movie, buf, &magic))
        break;
    }

  fclose (file);
  
  return magic;
}

static gboolean
bl_movie_load_all (BlMovie *movie)
{
  FILE     *file;
  gchar     buf[1024];
  guchar   *data;
  gboolean  header_parsed = FALSE;
  gboolean  magic         = FALSE;
  gint      line          = -1;
  gint      duration;
  gint      len;
  gint      i;

  g_return_val_if_fail (movie->frames == NULL && movie->n_frames == 0, FALSE);

  file = fopen (movie->filename, "r");
  if (!file)
    return FALSE;

  data = g_malloc (movie->width * movie->height);

  while (fget_line (buf, sizeof (buf), file))
    {
      if (!header_parsed)
        {
          if (!bl_movie_parse_header_line (movie, buf, &magic))
            {
              if (magic)
                header_parsed = TRUE;
              else
                break;

              movie->duration = 0;
            }
        }

      if (header_parsed)
        {
          len = strlen (buf);

          /* skip empty lines and comments */
          if (len == 0 || buf[0] == '#' || isspace (buf[0]))
            continue;

          if (line == -1)
            {
              if (buf[0] == '@')
                {
                  if (sscanf (buf+1, "%d", &duration) == 1 && duration > 0)
                    line = 0;
                }
            }
          else
            {
              if (buf[0] == '@' || len - 1 < movie->width)
                {
                  g_message ("Invalid frame, skipping.");
                  line = -1;
                }
              else
                {
                  for (i = 0; i < movie->width; i++)
                    data[movie->width * line + i] = (buf[i] == '1' ? 1 : 0);
                  if (++line == movie->height)
                    {
                      bl_movie_prepend_frame (movie, duration, data);
                      line = -1;
                    }
                }            
            }
        }
    }

  fclose (file);
 
  g_free (data);

  if (magic && movie->n_frames > 0)
    {
      movie->frames = g_list_reverse (movie->frames);
      return TRUE;
    }

  return FALSE;
}

static gboolean
bl_movie_parse_header_line (BlMovie     *movie,
                            const gchar *buf,
                            gboolean    *magic)
{
  const gchar *comment;
  gint len;

  if (buf[0] != '#')
    return FALSE;

  comment = buf + 1;
  while (isspace (*comment))
    comment++;

  if (g_strncasecmp (comment, "BlinkenLights Movie", 19) == 0)
    *magic = TRUE;
  
  if (!*magic)
    return FALSE;

  len = strlen (comment);
  
  if (!movie->name && 
      g_strncasecmp (comment, "name=", 5) == 0)
    {
      movie->name = g_strndup (comment + 5, len - 6);
    }
  else if (!movie->description && 
           g_strncasecmp (comment, "description=", 12) == 0)
    {
      movie->description = g_strndup (comment + 12, len - 13);
    }
  else if (!movie->duration && 
           g_strncasecmp (comment, "duration=", 9) == 0)
    {
      sscanf (comment + 9, "%d", &movie->duration);
    }

  return TRUE;
}

static void
bl_movie_prepend_frame (BlMovie       *movie, 
                        gint           duration, 
                        const guchar  *data)
{
  BlMovieFrame *frame;

  frame = g_new (BlMovieFrame, 1);

  frame->start = movie->duration;
  frame->data = g_malloc (movie->width * movie->height);
  memcpy (frame->data, data, movie->width * movie->height);

  movie->frames = g_list_prepend (movie->frames, frame);
  movie->n_frames++;
  movie->duration += duration;
}

GList * 
bl_movie_get_frame_at_time (BlMovie *movie,
                            GList   *seed,
                            gint     time)
{
  BlMovieFrame *frame;
  GList        *list;

  g_return_val_if_fail (movie != NULL, FALSE);
  g_return_val_if_fail (BL_IS_MOVIE (movie), FALSE);

  if (!movie->frames)
    return NULL;

  list = seed ? seed : movie->frames;

  frame = (BlMovieFrame *) list->data;

  while (time > frame->start && list->next)
    {
      list = list->next;
      frame = (BlMovieFrame *) list->data;
    }

  while (list->prev && time <= frame->start)
    {
      list = list->prev;
      frame = (BlMovieFrame *) list->data;
    }

  return list;
}

BlMovie *
bl_movie_new (const gchar *filename)
{
  BlMovie *movie;
  time_t   mtime;

  g_return_val_if_fail (filename != NULL, NULL);

  if (!bl_movie_check_file (filename, &mtime))
    return NULL;

  movie = BL_MOVIE (gtk_object_new (BL_TYPE_MOVIE, NULL));

  movie->filename = g_strdup (filename);
  movie->mtime    = mtime;

  if (!bl_movie_load_info (movie))
    {
      gtk_object_sink (GTK_OBJECT (movie));
      return NULL;
    }

  return movie;
}

gboolean
bl_movie_load (BlMovie *movie) 
{
  time_t mtime;

  g_return_val_if_fail (movie != NULL, FALSE);
  g_return_val_if_fail (BL_IS_MOVIE (movie), FALSE);

  if (!bl_movie_check_file (movie->filename, &mtime))
    return FALSE;  

  if (movie->loaded)
    {
      if (movie->mtime == mtime)
        return TRUE;

      bl_movie_destroy_data (movie);
    }

  movie->mtime = mtime;
  movie->loaded = bl_movie_load_all (movie);

  return movie->loaded;
}
