/* b2mng
 * Creates MNG animations from Blinkenlights movies.
 *
 * Copyright (C) 2002-2004  Sven Neumann <sven@gimp.org>
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <zlib.h>

#define MNG_SUPPORT_WRITE
#define MNG_ACCESS_CHUNKS
#include <libmng.h>

#include <blib/blib.h>

#include "bmovie-mng.h"


typedef struct
{
  FILE   *stream;
} UserData;

static mng_bool
mng_cb_writedata (mng_handle  hMNG,
                  mng_ptr     pBuf,
                  mng_uint32  iSize,
                  mng_uint32 *iWritten)
{
  UserData *data = (UserData *) mng_get_userdata (hMNG);

  *iWritten = fwrite (pBuf, 1, iSize, data->stream);

  return MNG_TRUE;
}

static mng_ptr
mng_cb_alloc (mng_size_t iSize)
{
  return (mng_ptr) g_malloc0 (iSize);
}

static void
mng_cb_free (mng_ptr    pPtr,
             mng_size_t iSize)
{
  g_free (pPtr);
}

static mng_bool
mng_cb_openstream (mng_handle hMNG)
{
  return MNG_TRUE;
}

static mng_bool
mng_cb_closestream (mng_handle hMNG)
{
  return MNG_TRUE;
}

static gpointer
zlib_compress_frame (const guchar *data,
                     gint          width,
                     gint          height,
                     gint          rowstride,
                     gulong       *len)
{
  static guchar *z_in  = NULL;
  static guchar *z_out = NULL;
  static gulong  z_len = 0;
  gulong  size;
  gint    row;
  guchar *dest;

  size = (width + 1) * height;

  if (size > z_len)
    {
      z_len = size;
      z_in  = g_realloc (z_in,  z_len);
      z_out = g_realloc (z_out, z_len + 20);
    }

  *len = z_len + 20;

  for (row = 0, dest = z_in; row < height; row++)
    {
      *dest = 0;  /* zero filter-byte */
      dest++;

      memcpy (dest, data, width);

      data += rowstride;
      dest += width;
    }

  compress2 (z_out, len, z_in, z_len, Z_BEST_COMPRESSION);

  return z_out;
}

static void
mng_put_simple_frame_data (mng_handle    hMNG,
                           BMovie       *movie,
                           BRectangle   *rect,
                           const guchar *data)
{
  gpointer    idat;
  gulong      len;
  BRectangle  area = { 0, 0, movie->width, movie->height };

  if (rect)
    area = *rect;

  if (area.w < 1 || area.h < 1)
    return;

  mng_putchunk_defi (hMNG,
                     0,
                     MNG_DONOTSHOW_VISIBLE,
                     MNG_ABSTRACT,
                     MNG_TRUE, area.x, area.y,
                     MNG_FALSE, 0, 0, 0, 0);

  if (mng_putchunk_ihdr (hMNG,
                         area.w, area.h, 8, MNG_COLORTYPE_GRAY,
                         MNG_COMPRESSION_DEFLATE, MNG_FILTER_ADAPTIVE,
                         MNG_INTERLACE_NONE))
     g_warning ("mng_putchunk_ihdr() failed");

  idat = zlib_compress_frame (data + area.y * movie->width + area.x,
                              area.w, area.h, movie->width, &len);

  mng_putchunk_idat (hMNG, len, idat);
  mng_putchunk_iend (hMNG);
}

static void
mng_put_image_data (mng_handle  hMNG,
                    FILE       *file,
                    gint       *width,
                    gint       *height)
{
  guint32  chunksize;
  gchar    chunkname[9];
  guchar  *chunkbuffer;

  while (! feof (file))
    {
      chunkbuffer = NULL;

      if (fread (&chunksize, 1, 4, file) != 4)
        break;
      chunksize = g_ntohl (chunksize);

      if (fread (chunkname, 1, 4, file) != 4)
        break;
      chunkname[4] = 0;

      if (chunksize > 0)
        {
          chunkbuffer = g_new (guchar, chunksize);

          if (fread (chunkbuffer, 1, chunksize, file) != chunksize)
            break;
        }

      if (strncmp (chunkname, "IHDR", 4) == 0)
        {
          guint32 chunkwidth  = g_ntohl (*((guint32 *) chunkbuffer));
          guint32 chunkheight = g_ntohl (*((guint32 *) chunkbuffer + 1));

          mng_putchunk_ihdr (hMNG,
                             chunkwidth, chunkheight,
                             chunkbuffer[8],  chunkbuffer[9],
                             chunkbuffer[10], chunkbuffer[11],
                             chunkbuffer[12]);

          if (width)  *width  = chunkwidth;
          if (height) *height = chunkheight;
        }
      else if (strncmp (chunkname, "JHDR", 4) == 0)
        {
          guint32 chunkwidth  = g_ntohl (*((guint32 *) chunkbuffer));
          guint32 chunkheight = g_ntohl (*((guint32 *) chunkbuffer + 1));

          mng_putchunk_jhdr (hMNG,
                             chunkwidth, chunkheight,
                             chunkbuffer[8],  chunkbuffer[9],
                             chunkbuffer[10], chunkbuffer[11],
                             0, 0, 0, 0);

          if (width)  *width  = chunkwidth;
          if (height) *height = chunkheight;
        }
      else if (strncmp (chunkname, "IDAT", 4) == 0)
        {
          mng_putchunk_idat (hMNG,
                             chunksize, chunkbuffer);
        }
      else if (strncmp (chunkname, "JDAT", 4) == 0)
        {
          mng_putchunk_jdat (hMNG,
                             chunksize, chunkbuffer);
        }
      else if (strncmp (chunkname, "JSEP", 4) == 0)
        {
          mng_putchunk_jsep (hMNG);
        }
      else if (strncmp (chunkname, "IEND", 4) == 0)
        {
          mng_putchunk_iend (hMNG);
        }
      else if (strncmp (chunkname, "PLTE", 4) == 0)
        {
          mng_putchunk_plte (hMNG,
                             chunksize / 3, (mng_palette8e *) chunkbuffer);
        }
      else if (strncmp (chunkname, "tRNS", 4) == 0)
        {
          mng_putchunk_trns (hMNG,
                             0, 0, 3, chunksize,
                             (mng_uint8 *) chunkbuffer, 0, 0, 0, 0,
                             0,
                             (mng_uint8 *) chunkbuffer);
        }

      if (chunksize > 0)
        g_free (chunkbuffer);

      /* read 4 bytes after the chunk */
      fread (chunkname, 1, 4, file);
    }
}

static gboolean
mng_put_image (mng_handle    hMNG,
               guint16       id,
               const gchar  *filename,
               gboolean      visible,
               gint         *width,
               gint         *height,
               GError      **error)
{
  FILE  *file;
  gchar  buf[8];

  g_return_val_if_fail (filename != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  file = fopen (filename, "rb");

  if (! file)
    {
      g_set_error (error,
                   G_FILE_ERROR,
                   g_file_error_from_errno (errno),
                   "Error reading file '%s': %s",
                   filename, g_strerror (errno));
      return FALSE;
    }

  if (fread (buf, 1, 8, file) == 8 &&
      (strncmp (buf, "\211PNG\r\n\032\n", 8) == 0 ||
       strncmp (buf, "\213JNG\r\n\032\n", 8) == 0))
    {
      mng_putchunk_defi (hMNG,
                         id,
                         (visible ?
                          MNG_DONOTSHOW_VISIBLE : MNG_DONOTSHOW_NOTVISIBLE),
                         MNG_ABSTRACT,
                         MNG_FALSE, 0, 0,
                         MNG_FALSE, 0, 0, 0, 0);

      mng_put_image_data (hMNG, file, width, height);

      fclose (file);
      return TRUE;
    }
  else
    {
      g_set_error (error, 0, 0, "Unsupported fileformat in '%s'", filename);

      fclose (file);
      return FALSE;
    }
}

static void
mng_put_loop (mng_handle  hMNG,
              gint        loops)
{
  mng_putchunk_save (hMNG, MNG_TRUE, 0, 0);

  if (loops < 0)
    mng_putchunk_term (hMNG,
                       MNG_TERMACTION_REPEAT, MNG_ITERACTION_LASTFRAME,
                       0, 0x7fffffff);
  else if (loops > 1)
    mng_putchunk_term (hMNG,
                       MNG_TERMACTION_REPEAT, MNG_ITERACTION_LASTFRAME,
                       0, loops);
  else
    mng_putchunk_term (hMNG,
                       MNG_TERMACTION_LASTFRAME, MNG_ITERACTION_LASTFRAME,
                       0, 0);

  mng_putchunk_seek (hMNG, 0, MNG_NULL);
}

static void
b_movie_mng_header (BMovie     *movie,
                    BTheme     *theme,
                    mng_handle  hMNG)
{
  if (mng_putchunk_mhdr (hMNG,
                         theme ? theme->width  : movie->width,
                         theme ? theme->height : movie->height,
                         1000,
                         0,
                         movie->n_frames,
                         movie->duration,
                         (MNG_SIMPLICITY_VALID           |
                          MNG_SIMPLICITY_SIMPLEFEATURES  |
                          MNG_SIMPLICITY_COMPLEXFEATURES |
                          MNG_SIMPLICITY_JNG)))
    g_warning ("mng_putchunk_mhdr() failed");

  mng_putchunk_text (hMNG,
                     5, "Title", 19, "Blinkenlights Movie");
}

static void
b_movie_write_simple_mng (BMovie     *movie,
                          gint        loops,
                          mng_handle  hMNG)
{
  GList       *list  = movie->frames;
  BMovieFrame *frame = list->data;
  gint         delay = frame->duration;

  if (mng_putchunk_back (hMNG,
                         0, 0, 0, MNG_BACKGROUNDCOLOR_MANDATORY,
                         0, MNG_BACKGROUNDIMAGE_NOTILE))
    g_warning ("mng_putchunk_back() failed");

  mng_put_loop (hMNG, loops);

  mng_putchunk_fram (hMNG,
                     MNG_FALSE, MNG_FRAMINGMODE_1,
                     0, MNG_NULL,
                     MNG_CHANGEDELAY_DEFAULT,
                     MNG_CHANGETIMOUT_NO,
                     MNG_CHANGECLIPPING_NO,
                     MNG_CHANGESYNCID_NO,
                     delay, 0, 0,
                     0, 0, 0, 0,
                     0, 0);
  mng_put_simple_frame_data (hMNG, movie, NULL, frame->data);

  for (list = list->next; list; list = list->next)
    {
      frame = list->data;

      if (frame->duration != delay)
        {
          delay = frame->duration;

          mng_putchunk_fram (hMNG,
                             MNG_FALSE, MNG_FRAMINGMODE_NOCHANGE,
                             0, MNG_NULL,
                             MNG_CHANGEDELAY_DEFAULT,
                             MNG_CHANGETIMOUT_NO,
                             MNG_CHANGECLIPPING_NO,
                             MNG_CHANGESYNCID_NO,
                             delay, 0, 0,
                             0, 0, 0, 0,
                             0, 0);
        }

      mng_put_simple_frame_data (hMNG, movie, NULL, frame->data);
    }
}

typedef struct
{
  guint  id;
  gint   width;
  gint   height;
} Image;

typedef struct
{
  guint  image;
  gint   x;
  gint   y;
  gint   w;
  gint   h;
} Window;

static guint
window_hash (const Window *window)
{
  return (g_int_hash (&window->image) +
          g_int_hash (&window->x)     +
          g_int_hash (&window->y)     +
          g_int_hash (&window->w)     +
          g_int_hash (&window->h));
}

static gboolean
window_equal (const Window *a,
              const Window *b)
{
  return (a->image == b->image &&
          a->x     == b->x     &&
          a->y     == b->y     &&
          a->w     == b->w     &&
          a->h     == b->h);
}

static Image *
load_image (mng_handle   hMNG,
            GHashTable  *images,
            guint       *next_id,
            const gchar *filename,
            gboolean     visible)
{
  Image *image = g_hash_table_lookup (images, filename);

  if (! image)
    {
      GError *error  = NULL;
      gint    width  = 0;
      gint    height = 0;
      guint   id     = *next_id;

      if (! mng_put_image (hMNG,
                           id, filename, visible, &width, &height, &error))
        {
          g_warning ("%s\n", error->message);
          g_error_free (error);
          return FALSE;
        }

      image = g_new0 (Image, 1);
      image->id     = id;
      image->width  = width;
      image->height = height;

      g_hash_table_insert (images, (gpointer) filename, image);
      *next_id = id + 1;
    }

  return image;
}

static guint
load_window (mng_handle   hMNG,
             GHashTable  *windows,
             guint       *next_id,
             Image       *image,
             BWindow     *window)
{
  Window  win = { image->id,
                  window->src_x,
                  window->src_y,
                  window->rect.w,
                  window->rect.h };

  guint id = GPOINTER_TO_UINT (g_hash_table_lookup (windows, &win));

  if (! id)
    {
      if (win.x == 0 &&
          win.y == 0 &&
          win.w == image->width &&
          win.h == image->height)
        {
          id = image->id;
        }
      else
        {
          id = *next_id;
          *next_id = id + 1;

          mng_putchunk_defi (hMNG,
                             id,
                             MNG_DONOTSHOW_NOTVISIBLE,
                             MNG_ABSTRACT,
                             MNG_FALSE, 0, 0,
                             MNG_FALSE, 0, 0, 0, 0);

          mng_putchunk_basi (hMNG,
                             win.w, win.h,
                             MNG_BITDEPTH_8,
                             MNG_COLORTYPE_RGBA,
                             MNG_COMPRESSION_DEFLATE,
                             MNG_FILTER_ADAPTIVE,
                             MNG_INTERLACE_NONE,
                             0x0, 0x0, 0x0, 0x0,
                             MNG_VIEWABLE);

          mng_putchunk_iend (hMNG);

          mng_putchunk_past (hMNG,
                             id,
                             MNG_TARGET_ABSOLUTE, 0, 0,
                             1);
          mng_putchunk_past_src (hMNG,
                                 0,
                                 image->id,
                                 MNG_COMPOSITE_REPLACE,
                                 MNG_ORIENTATION_SAME,
                                 MNG_OFFSET_ABSOLUTE, - win.x, - win.y,
                                 MNG_BOUNDARY_ABSOLUTE, 0, win.w, 0, win.h);
        }

      g_hash_table_insert (windows,
                           g_memdup (&win, sizeof (Window)),
                           GUINT_TO_POINTER (id));
    }

  return id;
}

static void
mng_load_theme_frame (mng_handle    hMNG,
                      BTheme       *theme,
                      GHashTable   *images,
                      GHashTable   *windows,
                      guint        *next_id,
                      const guchar *frame_data)
{
  GList *list;

  for (list = theme->overlays; list; list = list->next)
    {
      BOverlay *overlay = list->data;

      if (overlay->image)
        {
          GList *iter;
          Image *image;
          guint  value;

          image = load_image (hMNG, images, next_id, overlay->image, FALSE);

          for (iter = overlay->windows; iter; iter = iter->next)
            {
              BWindow *window = iter->data;

              value = frame_data[window->column +
                                 window->row * theme->columns];

              if (value)
                {
                  window += (value * theme->maxval) / 256;

                  load_window (hMNG, windows, next_id, image, window);
                }
            }
        }
    }
}

static void
mng_put_window (mng_handle  hMNG,
                guint       id,
                gint        x,
                gint        y,
                gint        repeat,
                gint        dx,
                gint        dy)
{
  mng_putchunk_move (hMNG, id, id, MNG_LOCATION_ABSOLUTE, x, y);
  mng_putchunk_show (hMNG, MNG_FALSE, id, id, MNG_SHOWMODE_0);

  switch (repeat)
    {
    case 0:
      return;
    case 1:
      break;
    default:
      mng_putchunk_loop (hMNG, 0, repeat,
                         MNG_TERMINATION_DETERMINISTIC_C,
                         0, 0, 0, MNG_NULL);
      break;
    }

  mng_putchunk_move (hMNG, id, id, MNG_LOCATION_RELATIVE, dx, dy);
  mng_putchunk_show (hMNG, MNG_FALSE, id, id, MNG_SHOWMODE_0);

  if (repeat > 1)
    mng_putchunk_endl (hMNG, 0);
}

static void
mng_put_theme_frame (mng_handle    hMNG,
                     BTheme       *theme,
                     GHashTable   *images,
                     GHashTable   *windows,
                     const guchar *frame_data,
                     BRectangle   *clip)
{
  GList *list;
  GList *iter;

  for (list = theme->overlays; list; list = list->next)
    {
      BOverlay *overlay  = list->data;
      Image    *image    = NULL;
      Window    last_win = { 0, 0, 0, 0, 0 };
      gint      x        = 0;
      gint      y        = 0;
      gint      dx       = 0;
      gint      dy       = 0;
      gint      repeat   = 0;
      guint     id;

      if (overlay->image)
        image = g_hash_table_lookup (images, overlay->image);

      if (! image)
        continue;

      for (iter = overlay->windows; iter; iter = iter->next)
        {
          BWindow *window   = iter->data;
          guint    value    = frame_data[window->column +
                                         window->row * theme->columns];

          window += (value * theme->maxval) / 256;

          if (value)
            {
              Window  win = { image->id,
                              window->src_x,  window->src_y,
                              window->rect.w, window->rect.h };

              if (clip && ! b_rectangle_intersect (&window->rect, clip, NULL))
                continue;

              if (window_equal (&win, &last_win))
                {
                  if (repeat)
                    {
                      if ((window->rect.x - x == (repeat + 1) * dx) &&
                          (window->rect.y - y == (repeat + 1) * dy))
                        {
                          repeat++;
                          continue;
                        }
                    }
                  else
                    {
                      dx = window->rect.x - x;
                      dy = window->rect.y - y;

                      repeat++;
                      continue;
                    }
                }

              if (last_win.image)
                {
                  id = GPOINTER_TO_UINT (g_hash_table_lookup (windows,
                                                              &last_win));
                  mng_put_window (hMNG, id, x, y, repeat, dx, dy);

                  repeat = 0;
                }

              memcpy (&last_win, &win, sizeof (Window));

              x = window->rect.x;
              y = window->rect.y;
            }
        }

      if (last_win.image)
        {
          id = GPOINTER_TO_UINT (g_hash_table_lookup (windows, &last_win));
          mng_put_window (hMNG, id, x, y, repeat, dx, dy);
        }
    }
}

static void
b_movie_write_themed_mng (BMovie     *movie,
                          BTheme     *theme,
                          gint        loops,
                          mng_handle  hMNG)
{
  GHashTable  *images;
  GHashTable  *windows;
  GList       *list;
  BMovieFrame *frame;
  BRectangle   prev_clip = { 0, 0, theme->width, theme->height };
  guint8       mandatory = MNG_BACKGROUNDCOLOR_MANDATORY;
  guint        id        = 0;
  guint        next_id   = 1;

  images = g_hash_table_new (g_str_hash, g_str_equal);

  if (theme->bg_image)
    {
      Image *image = load_image (hMNG,
                                 images, &next_id, theme->bg_image, TRUE);
      if (image)
        {
          id = image->id;
          mandatory |= MNG_BACKGROUNDIMAGE_MANDATORY;
        }
    }

  if (mng_putchunk_back (hMNG,
                         ((theme->bg_color.r << 8) | theme->bg_color.r),
                         ((theme->bg_color.g << 8) | theme->bg_color.g),
                         ((theme->bg_color.b << 8) | theme->bg_color.b),
                         mandatory, id, MNG_BACKGROUNDIMAGE_NOTILE))
    g_warning ("mng_putchunk_back() failed");

  windows = g_hash_table_new ((GHashFunc)  window_hash,
                              (GEqualFunc) window_equal);

  for (list = movie->frames; list; list = list->next)
    {
      frame = list->data;

      mng_load_theme_frame (hMNG,
                            theme, images, windows, &next_id, frame->data);
    }

  mng_put_loop (hMNG, loops);

  list  = movie->frames;
  frame = list->data;

  mng_putchunk_fram (hMNG,
                     MNG_FALSE,
                     MNG_FRAMINGMODE_4,
                     0, MNG_NULL,
                     MNG_CHANGEDELAY_DEFAULT,
                     MNG_CHANGETIMOUT_NO,
                     MNG_CHANGECLIPPING_NO,
                     MNG_CHANGESYNCID_NO,
                     frame->duration, 0,
                     MNG_BOUNDARY_ABSOLUTE, 0, 0, 0, 0,
                     0, 0);
  mng_put_theme_frame (hMNG, theme, images, windows, frame->data, NULL);

  for (list = list->next; list; list = list->next)
    {
      BMovieFrame *prev_frame;
      BRectangle   clip;
      gboolean     delay_changed;
      gboolean     clip_changed;

      prev_frame = list->prev->data;
      frame      = list->data;

      b_theme_frame_diff_boundary (theme,
                                   prev_frame->data, frame->data, &clip);

      delay_changed = prev_frame->duration != frame->duration;
      clip_changed  = (clip.x != prev_clip.x || clip.y != prev_clip.y ||
                       clip.w != prev_clip.w || clip.h != prev_clip.h);

      mng_putchunk_fram (hMNG,
                         !delay_changed && !clip_changed,
                         MNG_FRAMINGMODE_NOCHANGE,
                         0, MNG_NULL,
                         (delay_changed ?
                          MNG_CHANGEDELAY_DEFAULT : MNG_CHANGEDELAY_NO),
                         MNG_CHANGETIMOUT_NO,
                         (clip_changed ?
                          MNG_CHANGECLIPPING_DEFAULT : MNG_CHANGECLIPPING_NO),
                         MNG_CHANGESYNCID_NO,
                         frame->duration, 0,
                         MNG_BOUNDARY_ABSOLUTE,
                         clip.x, clip.x + clip.w, clip.y, clip.y + clip.h,
                         0, 0);
      mng_put_theme_frame (hMNG, theme, images, windows, frame->data, &clip);

      prev_clip = clip;
    }
}

gboolean
b_movie_save_as_mng (BMovie  *movie,
                     BTheme  *theme,
                     FILE    *stream,
                     gint     loops,
                     GError **error)
{
  mng_handle   hMNG;
  UserData     data;

  g_return_val_if_fail (B_IS_MOVIE (movie), FALSE);
  g_return_val_if_fail (theme == NULL || B_IS_THEME (theme), FALSE);
  g_return_val_if_fail (stream != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (theme && theme->channels != 1)
    {
      g_set_error (error, 0, 0,
                   "Sorry, themes with channel != 1 are not (yet) supported.");
      return FALSE;
    }

  if (theme &&
      (movie->width != theme->columns || movie->height != theme->rows))
    {
      g_set_error (error, 0, 0,
                   "Movie and theme dimensions do not match.");
      return FALSE;
    }

  data.stream = stream;

  hMNG = mng_initialize ((mng_ptr) &data, mng_cb_alloc, mng_cb_free, MNG_NULL);
  if (! hMNG)
    goto error;

  if (mng_setcb_openstream  (hMNG, mng_cb_openstream)  ||
      mng_setcb_closestream (hMNG, mng_cb_closestream) ||
      mng_setcb_writedata   (hMNG, mng_cb_writedata))
    goto error;

  if (mng_create (hMNG))
    goto error;

  b_movie_mng_header (movie, theme, hMNG);

  if (movie->frames)
    {
      if (theme)
        b_movie_write_themed_mng (movie, theme, loops, hMNG);
      else
        b_movie_write_simple_mng (movie, loops, hMNG);
    }

  if (mng_putchunk_mend (hMNG))
    goto error;

  if (mng_write (hMNG))
    goto error;

  mng_cleanup (&hMNG);

  return TRUE;

 error:
  g_set_error (error, 0, 0, "Unexpected problems creating the MNG.");
  return FALSE;
}
