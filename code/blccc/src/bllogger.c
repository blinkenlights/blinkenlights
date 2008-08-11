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

#include <stdio.h>
#include <errno.h>
#include <time.h>

#include <blib/blib.h>

#include "bltypes.h"

#include "bllogger.h"


#define LOG_INDENT 2


static void  bl_logger_class_init (BlLoggerClass *klass);
static void  bl_logger_init       (BlLogger      *view);
static void  bl_logger_finalize   (GObject       *object);

static BObjectClass *parent_class = NULL;


GType
bl_logger_get_type (void)
{
  static GType logger_type = 0;

  if (!logger_type)
    {
      static const GTypeInfo logger_info =
      {
        sizeof (BlLoggerClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) bl_logger_class_init,
	NULL,		/* class_finalize */
	NULL,		/* class_data     */
	sizeof (BlLogger),
	0,              /* n_preallocs    */
	(GInstanceInitFunc) bl_logger_init,
      };

      logger_type = g_type_register_static (B_TYPE_OBJECT, 
                                            "BlLogger",
                                            &logger_info, 0);
    }
  
  return logger_type;
}

static void
bl_logger_class_init (BlLoggerClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->finalize = bl_logger_finalize;
}

static void
bl_logger_init (BlLogger *logger)
{
  logger->writer = NULL;
}

static void
bl_logger_finalize (GObject *object)
{
  BlLogger *logger = BL_LOGGER (object);

  if (logger->writer)
    {
      b_writer_free (logger->writer);
      fclose (logger->stream);

      logger->writer = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

BlLogger *
bl_logger_new_from_file (const gchar  *filename,
                         GError      **error)
{
  BlLogger *logger;
  FILE     *stream;

  g_return_val_if_fail (filename != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  stream = fopen (filename, "a");
  if (!stream)
    {
      g_set_error (error, 0, 0, g_strerror (errno));
      return NULL;
    }

  logger = BL_LOGGER (g_object_new (BL_TYPE_LOGGER,
                                    "filename", filename, NULL));

  logger->stream = stream;
  logger->writer = b_writer_new (stream, LOG_INDENT);

  return logger;
}

static void
bl_logger_get_time (BlLogger *logger)
{
  time_t     now = time (NULL);
  struct tm *tm  = gmtime (&now);

  g_snprintf (logger->year,   8, "%d", tm->tm_year + 1900);
  g_snprintf (logger->month,  8, "%d", tm->tm_mon  + 1);
  g_snprintf (logger->day,    8, "%d", tm->tm_mday);
  g_snprintf (logger->hour,   8, "%d", tm->tm_hour);
  g_snprintf (logger->minute, 8, "%d", tm->tm_min);
  g_snprintf (logger->second, 8, "%d", tm->tm_sec);
}

void
bl_logger_start_module (BlLogger *logger,
                        BModule  *module)
{
  gchar *title;
  gchar *description;
  gchar *author;

  g_return_if_fail (BL_IS_LOGGER (logger));
  g_return_if_fail (B_IS_MODULE (module));

  bl_logger_get_time (logger);

  b_write_open_tag (logger->writer, "start",
                    "tz",     "UTC",
                    "year",   logger->year,
                    "month",  logger->month,
                    "day",    logger->day,
                    "hour",   logger->hour,
                    "minute", logger->minute,
                    "second", logger->second,
                    NULL);

  b_module_describe (module, &title, &description, &author);

  if (title)
    {
      b_write_element (logger->writer, "title", title, NULL);
      g_free (title);
    }
  if (description)
    {
      b_write_element (logger->writer, "description", description, NULL);
      g_free (description);
    }
  if (author)
    {
      b_write_element (logger->writer, "author", author, NULL);
      g_free (author);
    }

  if (B_IS_MOVIE_PLAYER (module))
    {
      BMoviePlayer *player = B_MOVIE_PLAYER (module);

      if (player->filename)
        b_write_element (logger->writer, "filename", player->filename, NULL);
    }

  b_write_close_tag (logger->writer, "start");

  fflush (logger->stream);
}

void
bl_logger_stop (BlLogger *logger)
{
  g_return_if_fail (BL_IS_LOGGER (logger));

  bl_logger_get_time (logger);

  b_write_element (logger->writer, "stop", NULL,
                   "tz",     "UTC",
                   "year",   logger->year,
                   "month",  logger->month,
                   "day",    logger->day,
                   "hour",   logger->hour,
                   "minute", logger->minute,
                   "second", logger->second,
                   NULL);

  fflush (logger->stream);
}
