/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2002  The Blinkenlights Crew
 *                          Sven Neumann <sven@gimp.org>
 *                          Michael Natterer <mitch@gimp.org>
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

#include <glib-object.h>

#include "btypes.h"
#include "bmovie.h"
#include "bobject.h"
#include "bparser.h"

/*  it's bad style to include our own subclasses, but...  */
#include "bmovie-blm.h"
#include "bmovie-bml.h"
#include "bmovie-gif.h"


static void      b_movie_class_init          (BMovieClass  *class);
static void      b_movie_init                (BMovie       *movie);
static void      b_movie_finalize            (GObject      *object);
static void      b_movie_finalize_data       (BMovie       *movie,
                                              gboolean      all);
static void      b_movie_nullify_data        (BMovie       *movie,
                                              gboolean      all);
static gboolean  b_movie_load_info           (BMovie       *movie,
                                              GIOChannel   *io,
                                              GError      **error);
static gboolean  b_movie_load_all            (BMovie       *movie,
                                              GIOChannel   *io,
                                              GError      **error);
static BMovie *  b_movie_new_from_io_channel (GIOChannel   *io,
                                              const gchar  *filename,
                                              const gchar  *name,
                                              gboolean      lazy,
                                              GError      **error);
static GType     b_movie_type_from_io_channel(GIOChannel   *io,
                                              GError      **error);


static BObjectClass *parent_class = NULL;


GType
b_movie_get_type (void)
{
  static GType movie_type = 0;

  if (!movie_type)
    {
      static const GTypeInfo movie_info =
      {
        sizeof (BMovieClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_movie_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BMovie),
        0,              /* n_preallocs */
        (GInstanceInitFunc) b_movie_init,
      };

      movie_type = g_type_register_static (B_TYPE_OBJECT,
                                           "BMovie", &movie_info, 0);
    }

  return movie_type;
}

static void
b_movie_class_init (BMovieClass *class)
{
  GObjectClass *object_class;

  parent_class = g_type_class_peek_parent (class);
  object_class = G_OBJECT_CLASS (class);

  object_class->finalize = b_movie_finalize;

  class->load_info = NULL;
  class->load_all  = NULL;
}

static void
b_movie_init (BMovie *movie)
{
  movie->width      = 0;
  movie->height     = 0;
  movie->maxval     = 0;
  movie->channels   = 1;

  movie->load_count = 0;

  b_movie_nullify_data (movie, TRUE);
}

static void
b_movie_finalize (GObject *object)
{
  BMovie *movie = B_MOVIE (object);

  b_movie_finalize_data (movie, TRUE);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
b_movie_finalize_data (BMovie   *movie,
                        gboolean  all)
{
  BMovieFrame *frame;
  GList       *list;

  g_return_if_fail (B_IS_MOVIE (movie));

  if (all)
    {
      g_free (movie->title);
      g_free (movie->description);
      g_free (movie->creator);
      g_free (movie->author);
      g_free (movie->email);
      g_free (movie->url);
    }

  for (list = movie->frames; list; list = g_list_next (list))
    {
      frame = (BMovieFrame *) list->data;

      g_free (frame->data);
      g_free (frame);
    }
  g_list_free (movie->frames);

  b_movie_nullify_data (movie, all);
}

static void
b_movie_nullify_data (BMovie   *movie,
                      gboolean  all)
{
  movie->duration = 0;
  movie->n_frames = 0;
  movie->frames   = NULL;

  if (all)
    {
      movie->title       = NULL;
      movie->description = NULL;
      movie->creator     = NULL;
      movie->author      = NULL;
      movie->email       = NULL;
      movie->url         = NULL;
      movie->loop        = FALSE;
    }
}

static gboolean
b_movie_load_info (BMovie      *movie,
                   GIOChannel  *io,
                   GError     **error)
{
  if (B_MOVIE_GET_CLASS (movie)->load_info)
    return B_MOVIE_GET_CLASS (movie)->load_info (movie, io, error);

  g_warning ("b_movie_load_info() unimplemented");
  return FALSE;
}

static gboolean
b_movie_load_all (BMovie      *movie,
                  GIOChannel  *io,
                  GError     **error)
{
  gboolean success = FALSE;

  b_movie_finalize_data (movie, TRUE);

  if (B_MOVIE_GET_CLASS (movie)->load_all)
    success = B_MOVIE_GET_CLASS (movie)->load_all (movie, io, error);
  else
    g_warning ("b_movie_load_all() unimplemented");

  if (success)
    {
      if (movie->n_frames > 0)
        {
          movie->frames = g_list_reverse (movie->frames);
        }
      else
        {
          g_set_error (error, 0, 0,
                       "Couldn't find any valid frames in the input.");
          success = FALSE;
        }
    }

  return success;
}

/**
 * b_movie_new_from_file:
 * @filename: the name of the file to load
 * @lazy_load: whether to do lazy-loading, i.e. only load the header
 * @error: location to store the error occuring, or %NULL to ignore errors
 *
 * Tries to load a #BMovie from the file pointed to by @filename. If
 * @lazy_load is %TRUE, only the header is loaded and no frames are stored.
 *
 * Return value: a newly allocated #BMovie object or %NULL if the load
 * failed
 **/
BMovie *
b_movie_new_from_file (const gchar  *filename,
                       gboolean      lazy_load,
                       GError      **error)
{
  BMovie     *movie = NULL;
  GIOChannel *io;
  gchar      *name;

  g_return_val_if_fail (filename != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  name = g_filename_to_utf8 (filename, -1, NULL, NULL, NULL);

  io = g_io_channel_new_file (filename, "r", error);
  if (! io)
    return NULL;

  g_io_channel_set_encoding (io, NULL, NULL);

  movie = b_movie_new_from_io_channel (io, filename, name, lazy_load, error);

  g_io_channel_unref (io);

  g_free (name);

  return movie;
}

/**
 * b_movie_new_from_fd:
 * @fd: a UNIX file descriptor
 * @error: location to store the error occuring, or %NULL to ignore errors
 *
 * Tries to load a #BMovie from the UNIX file descriptor @fd. Lazy
 * loading from file descriptors is not implemented since it doesn't
 * seem to make much sense.
 *
 * Return value: a newly allocated #BMovie object or %NULL if the load
 * failed
 **/
BMovie *
b_movie_new_from_fd (gint     fd,
                     GError **error)
{
  BMovie      *movie = NULL;
  GIOChannel  *io;
  const gchar *name;

  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (fd == 0)
    name = "<stdin>";
  else
    name = "<pipe>";

  io = g_io_channel_unix_new (fd);

  g_io_channel_set_encoding (io, NULL, NULL);

  movie = b_movie_new_from_io_channel (io, NULL, name, FALSE, error);

  g_io_channel_unref (io);

  return movie;
}

/**
 * b_movie_load:
 * @movie: a #BMovie object
 * @error: location to store the error occuring, or %NULL to ignore errors
 *
 * Assures that the frames of the @movie are loaded.
 *
 * Return value: %TRUE if the movie was loaded successfully.
 **/
gboolean
b_movie_load (BMovie  *movie,
              GError **error)
{
  g_return_val_if_fail (B_IS_MOVIE (movie), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (movie->load_count == 0)
    {
      const gchar *filename;
      GIOChannel  *io;
      gboolean     success;

      g_return_val_if_fail (movie->frames == NULL && movie->n_frames == 0,
                            FALSE);

      filename = b_object_get_filename (B_OBJECT (movie));
      if (!filename)
        {
          g_set_error (error, 0, 0,
                       "Cannot load a movie that has no filename");
          return FALSE;
        }


      io = g_io_channel_new_file (filename, "r", error);
      if (! io)
        return FALSE;

      g_io_channel_set_encoding (io, NULL, NULL);

      success = ((b_movie_type_from_io_channel (io, error) ==
                  G_TYPE_FROM_INSTANCE (movie)) &&
                 b_movie_load_all (movie, io, error));

      g_io_channel_unref (io);

      if (!success)
        return FALSE;
    }

  movie->load_count++;

  return TRUE;
}

/**
 * b_movie_unload:
 * @movie: a #BMovie object
 *
 * Unloads the frames of a movie.
 *
 * The movie object counts how many times you call b_movie_load() and
 * b_movie_unload() and only really unloads the frames if
 * b_movie_unload() was called as often as b_movie_load(). Note that
 * b_movie_new_from_file() and b_movie_new_from_fd() call
 * b_movie_load() unless lazy-loading was requested.
 **/
void
b_movie_unload (BMovie *movie)
{
  g_return_if_fail (B_IS_MOVIE (movie));
  g_return_if_fail (movie->load_count > 0);

  movie->load_count--;

  if (movie->load_count == 0)
    b_movie_finalize_data (movie, FALSE);
}

/**
 * b_movie_save:
 * @movie: a #BMovie object
 * @stream: a FILE stream ready for writing
 * @error: location to store the error occuring, or %NULL to ignore errors
 *
 * Saves a #BMovie object to a @stream.
 *
 * Return value: %TRUE on success or %FALSE otherwise
 **/
gboolean
b_movie_save (BMovie  *movie,
              FILE    *stream,
              GError **error)
{
  g_return_val_if_fail (B_IS_MOVIE (movie), FALSE);
  g_return_val_if_fail (stream != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  return b_movie_save_as (movie, G_TYPE_FROM_INSTANCE (movie), stream, error);
}

/**
 * b_movie_save_as:
 * @movie: a #BMovie object
 * @movie_type: the movie type to use for saving
 * @stream: a FILE stream ready for writing
 * @error: location to store the error occuring, or %NULL to ignore errors
 *
 * Saves a #BMovie object to a @stream and allows to specify the file
 * format to use.
 *
 * The format is specified by passing a type derived from
 * #B_TYPE_MOVIE as @movie_type. At the moment B_TYPE_MOVIE_BML,
 * B_TYPE_MOVIE_BLM and B_TYPE_MOVIE_GIF are supported types.
 *
 * Return value: %TRUE on success or %FALSE otherwise
 **/
gboolean
b_movie_save_as (BMovie *movie,
                 GType   movie_type,
                 FILE   *stream,
                 GError **error)
{
  BMovieClass *movie_class;
  gboolean     retval;

  g_return_val_if_fail (B_IS_MOVIE (movie), FALSE);
  g_return_val_if_fail (g_type_is_a (movie_type, B_TYPE_MOVIE), FALSE);
  g_return_val_if_fail (stream != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (! b_movie_load (movie, error))
    return FALSE;

  movie_class = g_type_class_ref (movie_type);

  retval = B_MOVIE_CLASS (movie_class)->save (movie, stream, error);

  g_type_class_unref (movie_class);

  b_movie_unload (movie);

  return retval;
}

/**
 * b_movie_get_frame_at_time:
 * @movie: a loaded #BMovie object
 * @seed: an optional GList pointer that can speed up the search
 * @time: the time in milliseconds
 *
 * Looks for the frame active after @time milliseconds.
 *
 * If you are calling this function subsequentially with increasing
 * time values, you can speed up the search by passing the last return
 * value as the @seed parameter.
 *
 * Return value: a GList pointer that has the frame as data or %NULL
 * if the time was out of the @movie's range.
 **/
GList *
b_movie_get_frame_at_time (BMovie *movie,
                           GList  *seed,
                           gint    time)
{
  BMovieFrame *frame;
  GList       *list;

  g_return_val_if_fail (B_IS_MOVIE (movie), FALSE);

  if (!movie->frames)
    return NULL;

  list = seed ? seed : movie->frames;

  frame = (BMovieFrame *) list->data;

  while (time > frame->start && list->next)
    {
      list = list->next;
      frame = (BMovieFrame *) list->data;
    }

  while (list->prev && time <= frame->start)
    {
      list = list->prev;
      frame = (BMovieFrame *) list->data;
    }

  return list;
}

/**
 * b_movie_prepend_frame:
 * @movie:
 * @duration:
 * @data:
 *
 * This is an internal function used by the movie loaders. It should
 * never be called from anywhere else.
 **/
void
b_movie_prepend_frame (BMovie       *movie,
                       gint          duration,
                       const guchar *data)
{
  BMovieFrame *frame;

  g_return_if_fail (B_IS_MOVIE (movie));
  g_return_if_fail (data != NULL);

  frame = g_new (BMovieFrame, 1);

  frame->start    = movie->duration;
  frame->duration = duration;
  frame->data     = g_memdup (data,
                              movie->width * movie->height * movie->channels);

  movie->frames = g_list_prepend (movie->frames, frame);
  movie->n_frames++;
  movie->duration += duration;
}

static BMovie *
b_movie_new_from_io_channel (GIOChannel   *io,
                             const gchar  *filename,
                             const gchar  *name,
                             gboolean      lazy,
                             GError      **error)
{
  BMovie   *movie;
  GType     movie_type;
  gboolean  success;

  movie_type = b_movie_type_from_io_channel (io, error);

  if (movie_type == G_TYPE_NONE)
    {
      if (error && !*error)
        g_set_error (error, 0, 0, "Unknown file type");
      return NULL;
    }

  movie = B_MOVIE (g_object_new (movie_type,
                                 "filename", filename,
                                 "name",     name,
                                 NULL));

  if (lazy)
    {
      success = b_movie_load_info (movie, io, error);
    }
  else
    {
      success = b_movie_load_all (movie, io, error);
      movie->load_count++;
    }

  if (!success)
    {
      g_object_unref (movie);
      movie = NULL;
    }

  if (movie && !movie->title)
    movie->title = g_strdup (b_object_get_name (B_OBJECT (movie)));

  return movie;
}

static GType
b_movie_type_from_io_channel (GIOChannel  *io,
                              GError     **error)
{
  GType        movie_type = G_TYPE_NONE;
  const gchar *magic      = NULL;
  gsize        magic_len  = 1;
  gsize        len        = 0;
  gchar        buf[1024];

  for (len = 0; len < sizeof (buf); len++)
    {
      GIOStatus  status;

      status = g_io_channel_read_chars (io, buf + len, 1, NULL, error);

      if (status == G_IO_STATUS_ERROR || status == G_IO_STATUS_EOF)
        return G_TYPE_NONE;

      if (movie_type == G_TYPE_NONE)
        {
          switch (buf[len])
            {
            case '#':
              movie_type = B_TYPE_MOVIE_BLM;
              magic = buf + len + 1;
              break;
            case '<':
              movie_type = B_TYPE_MOVIE_BML;
              magic = buf + len;
              break;
            case 'G':
              if (len == 0)
                movie_type = B_TYPE_MOVIE_GIF;
              break;
            }
        }
      else if (movie_type == B_TYPE_MOVIE_BLM)
        {
          if (magic_len == 1 && buf[len] == ' ')
            magic++;
          else
            magic_len++;

          if (magic_len == 20)
            {
              if (g_ascii_strncasecmp (magic, "BlinkenLights Movie", 19))
                return G_TYPE_NONE;
              else
                return B_TYPE_MOVIE_BLM;
            }
        }
      else if (movie_type == B_TYPE_MOVIE_GIF)
        {
          if (len < 5)
            continue;

          if (len == 5 &&
              (strncmp (buf, "GIF87a", 6) == 0 ||
               strncmp (buf, "GIF89a", 6) == 0))
            return B_TYPE_MOVIE_GIF;

          return G_TYPE_NONE;
        }
      else if (movie_type == B_TYPE_MOVIE_BML)
        {
          magic_len++;
          if (magic_len == 5)
            {
              if (strncmp (magic, "<?xml", 5))
                return G_TYPE_NONE;
            }
          else if (magic_len > 6 && buf[len-1] == '?' && buf[len] == '>')
            {
              gboolean  ret;
              gchar    *encoding = b_parse_encoding (magic, magic_len);

              ret = g_io_channel_set_encoding (io, encoding, error);
              g_free (encoding);

              return (ret ? B_TYPE_MOVIE_BML : G_TYPE_NONE);
            }
        }
    }

  return G_TYPE_NONE;
}
