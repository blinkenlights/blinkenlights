/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2002  The Blinkenlights Crew
 *                          Michael Natterer <mitch@gimp.org>
 *                          Stefan Schuermans <1stein@blinkenarea.org>
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

#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <glib/gstdio.h>
#include <glib-object.h>

#include <blib/blib.h>

#include "characters.h"


#define TIMEOUT_MIN 10

typedef enum
{
  SCROLL_UP,
  CURSOR_BLINK,
} AnimType;


enum
{
  PROP_0,
  PROP_STRING,
  PROP_FILE,
  PROP_DIR,
  PROP_TEXT_TIMEOUT,
  PROP_BLINK_TIMEOUT,
  PROP_SCROLL_TIMEOUT,
  PROP_INIT_BLINK_STEPS,
  PROP_BEGIN_BLINK_STEPS,
  PROP_END_BLINK_STEPS,
  PROP_EXIT_BLINK_STEPS,
  PROP_BG_COLOR,
  PROP_FG_COLOR,
  PROP_FONT_SIZE,
};


#define B_TYPE_TEXT            (b_type_text)
#define B_TEXT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_TEXT, BText))
#define B_TEXT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_TEXT, BTextClass))
#define B_IS_TEXT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_TEXT))
#define B_IS_TEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), B_TYPE_TEXT))

typedef struct _BText      BText;
typedef struct _BTextClass BTextClass;

struct _BText
{
  BModule       parent_instance;

  /* settings from config */
  gchar        *setting_string;
  gchar        *setting_file;
  gchar        *setting_dir;

  gint          setting_text_timeout; /* may be 0 to turn off waiting */
  gint          setting_blink_timeout;
  gint          setting_scroll_timeout; /* may be zero to turn off waiting */
  gint          setting_init_blink_steps;
  gint          setting_begin_blink_steps;
  gint          setting_end_blink_steps;
  gint          setting_exit_blink_steps;

  guchar        setting_bg_color;
  guchar        setting_fg_color;

  gint          setting_font_width;
  gint          setting_font_height;
  gint          setting_font_columns;
  gint          setting_font_lines;

  /* selected font */
  const ChFont *p_font;

  /* string to display, position in string, if last char was a newline char */
  gchar        *string;
  gchar        *cursor_pos;
  gboolean      newline;

  /* position on display */
  gint          cursor_x;
  gint          cursor_y;

  gint          player_device_id;

  /* timeout to use for tick procedure */
  gint          timeout;

  /* type of current animation (if anmin_steps > 0) */
  AnimType      anim;
  gint          anim_steps;

  /* if blinking before exit has been started */
  gboolean      exit_blink;

};

struct _BTextClass
{
  BModuleClass  parent_class;
};


static GType      b_text_get_type      (GTypeModule   *module);

static void       b_text_class_init    (BTextClass    *klass);
static void       b_text_init          (BText         *text);

static void       b_text_finalize      (GObject       *object);
static void       b_text_set_property  (GObject       *object,
                                        guint          property_id,
                                        const GValue  *value,
                                        GParamSpec    *pspec);
static gboolean   b_text_query         (gint           width,
                                        gint           height,
                                        gint           channels,
                                        gint           maxval);
static gboolean   b_text_prepare       (BModule       *module,
                                        GError       **error);
static void       b_text_relax         (BModule       *module);
static void       b_text_start         (BModule       *module);
static void       b_text_stop          (BModule       *module);
static void       b_text_event         (BModule       *module,
                                        BModuleEvent  *event);
static gint       b_text_tick          (BModule       *module);
static void       b_text_describe      (BModule       *module,
                                        const gchar  **title,
                                        const gchar  **description,
                                        const gchar  **author);


static BModuleClass *parent_class = NULL;
static GType         b_type_text  = 0;


G_MODULE_EXPORT gboolean
b_module_register (GTypeModule *module)
{
  b_text_get_type (module);
  return TRUE;
}

static GType
b_text_get_type (GTypeModule *module)
{
  if (! b_type_text)
    {
      static const GTypeInfo text_info =
      {
        sizeof (BTextClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_text_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BText),
        0,              /* n_preallocs */
        (GInstanceInitFunc) b_text_init,
      };

      b_type_text = g_type_module_register_type (module,
                                                 B_TYPE_MODULE, "BText",
                                                 &text_info, 0);
    }

  return b_type_text;
}

static void
b_text_class_init (BTextClass *klass)
{
  GObjectClass *object_class;
  BModuleClass *module_class;
  GParamSpec   *param_spec;

  object_class               = G_OBJECT_CLASS (klass);
  module_class               = B_MODULE_CLASS (klass);

  parent_class               = g_type_class_peek_parent (klass);

  object_class->finalize     = b_text_finalize;
  object_class->set_property = b_text_set_property;

  param_spec = g_param_spec_string ("string", NULL,
                                    "The string to draw (if no file is found).",
                                    "",
                                    G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_STRING, param_spec);

  param_spec = g_param_spec_string ("file", NULL,
                                    "The file to read the string to draw from (if dir contains no file).",
                                    "",
                                    G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_FILE, param_spec);

  param_spec = g_param_spec_string ("dir", NULL,
                                    "The directory to read (and then DELETE) the oldest file with a string to draw from.",
                                    "",
                                    G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_DIR, param_spec);

  param_spec = g_param_spec_int ("text_timeout", NULL,
				 "Timeout in miliseconds between displaying two characters.",
				 0, 1000, 200,
				 G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_TEXT_TIMEOUT, param_spec);

  param_spec = g_param_spec_int ("blink_timeout", NULL,
				 "Timeout in miliseconds how long the cursor is displayed when it blinks.",
				 TIMEOUT_MIN, 1000, 200,
				 G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_BLINK_TIMEOUT, param_spec);

  param_spec = g_param_spec_int ("scroll_timeout", NULL,
				 "Timeout in miliseconds between moving up the text one row when scrolling.",
				 0, 1000, 100,
				 G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_SCROLL_TIMEOUT, param_spec);

  param_spec = g_param_spec_int ("init_blink_steps", NULL,
				 "Number of times the cursor blinks at the begin of the text.",
				 0, 20, 2,
				 G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_INIT_BLINK_STEPS, param_spec);

  param_spec = g_param_spec_int ("begin_blink_steps", NULL,
				 "Number of times the cursor blinks at the begin of every line.",
				 0, 20, 2,
				 G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_BEGIN_BLINK_STEPS, param_spec);

  param_spec = g_param_spec_int ("end_blink_steps", NULL,
				 "Number of times the cursor blinks at the end of every line.",
				 0, 20, 2,
				 G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_END_BLINK_STEPS, param_spec);

  param_spec = g_param_spec_int ("exit_blink_steps", NULL,
				 "Number of times the cursor blinks at the end of the text.",
				 0, 80, 8,
				 G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_EXIT_BLINK_STEPS, param_spec);

  param_spec = g_param_spec_int ("bg_color", NULL,
				 "The background color.",
				 0, 255, 0,
				 G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_BG_COLOR, param_spec);

  param_spec = g_param_spec_int ("fg_color", NULL,
				 "The foreround color.",
				 0, 255, 255,
				 G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_FG_COLOR, param_spec);

  param_spec = g_param_spec_string ("font_size", NULL,
                                    "The size of the font to use (<width>x<height>[@<columns>x<lines>]).",
                                    "",
                                    G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_FONT_SIZE, param_spec);

  module_class->max_players = 1;

  module_class->query       = b_text_query;
  module_class->prepare     = b_text_prepare;
  module_class->relax       = b_text_relax;
  module_class->start       = b_text_start;
  module_class->stop        = b_text_stop;
  module_class->event       = b_text_event;
  module_class->tick        = b_text_tick;
  module_class->describe    = b_text_describe;
}

static void
b_text_init (BText *text)
{
  text->setting_string            = g_strdup ("");
  text->setting_file              = g_strdup ("");
  text->setting_dir               = g_strdup ("");

  text->setting_text_timeout      = 200;
  text->setting_blink_timeout     = 200;
  text->setting_scroll_timeout    = 100;
  text->setting_init_blink_steps  = 2;
  text->setting_begin_blink_steps = 2;
  text->setting_end_blink_steps   = 2;
  text->setting_exit_blink_steps  = 8;

  text->setting_bg_color          = 0;
  text->setting_fg_color          = text->parent_instance.maxval;

  text->setting_font_width        = -1;
  text->setting_font_height       = -1;
  text->setting_font_columns      = -1;
  text->setting_font_lines        = -1;

  text->string                    = g_strdup ("");

  text->anim_steps                = 0;
  text->player_device_id          = -1;
}

static void
b_text_finalize (GObject *object)
{
  BText * text = B_TEXT (object);

  g_free (text->setting_string);
  g_free (text->setting_file);
  g_free (text->setting_dir);

  g_free (text->string);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
b_text_set_property (GObject      *object,
                     guint         property_id,
                     const GValue *value,
                     GParamSpec   *pspec)
{
  BText *text = B_TEXT (object);
  gchar *str;
  gint width, height, columns, lines;

  switch (property_id)
    {
    case PROP_STRING:
      g_free (text->setting_string);
      text->setting_string = g_value_dup_string (value);
      break;

    case PROP_FILE:
      g_free (text->setting_file);
      text->setting_file = g_value_dup_string (value);
      break;

    case PROP_DIR:
      g_free (text->setting_dir);
      text->setting_dir = g_value_dup_string (value);
      break;

    case PROP_TEXT_TIMEOUT:
      text->setting_text_timeout = g_value_get_int (value);
      if (text->setting_text_timeout > 0 && text->setting_text_timeout < TIMEOUT_MIN)
        text->setting_text_timeout = TIMEOUT_MIN;
      break;

    case PROP_BLINK_TIMEOUT:
      text->setting_blink_timeout = g_value_get_int (value);
      break;

    case PROP_SCROLL_TIMEOUT:
      text->setting_scroll_timeout = g_value_get_int (value);
      if (text->setting_scroll_timeout > 0 && text->setting_scroll_timeout < TIMEOUT_MIN)
        text->setting_scroll_timeout = TIMEOUT_MIN;
      break;

    case PROP_INIT_BLINK_STEPS:
      text->setting_init_blink_steps = g_value_get_int (value);
      break;

    case PROP_BEGIN_BLINK_STEPS:
      text->setting_begin_blink_steps = g_value_get_int (value);
      break;

    case PROP_END_BLINK_STEPS:
      text->setting_end_blink_steps = g_value_get_int (value);
      break;

    case PROP_EXIT_BLINK_STEPS:
      text->setting_exit_blink_steps = g_value_get_int (value);
      break;

    case PROP_BG_COLOR:
      text->setting_bg_color = g_value_get_int (value);
      break;

    case PROP_FG_COLOR:
      text->setting_fg_color = g_value_get_int (value);
      break;

    case PROP_FONT_SIZE:
      str = g_value_dup_string (value);
      if (sscanf (str, "%dx%d@%dx%d", &width, &height, &columns, &lines) == 4
          && width >= 1 && height >= 1 && columns >= 1 && lines >= 1)
        {
          text->setting_font_width   = width;
          text->setting_font_height  = height;
          text->setting_font_columns = columns;
          text->setting_font_lines   = lines;
        }
      else if (sscanf (str, "%dx%d", &width, &height) == 2
		 && width >= 1 && height >= 1)
        {
          text->setting_font_width   = width;
          text->setting_font_height  = height;
          text->setting_font_columns = 1;
          text->setting_font_lines   = 1;
        }
      g_free (str);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
b_text_query (gint     width,
              gint     height,
              gint     channels,
              gint     maxval)
{
  /* check that a font is available that can at least display 3 characters on the display */
  const ChFont *p_font = selectChFont (3, 1, width, height);
  return p_font != pChFontNone && channels == 1; /* this module is only for one channel */
}

static gchar *get_oldest_filename_in_dir (gchar *dirname)
{
  GDir *dir;
  const gchar *cur_file_name;
  gchar *cur_name, *oldest_name = NULL;
  struct stat st;
  time_t oldest_mtime = 0;

  /* find oldest file in dir */
  if ((dir = g_dir_open (dirname, 0, NULL)))
    {
      while ((cur_file_name = g_dir_read_name (dir)))
        {
          /* construct complete filename */
          cur_name = (gchar *)g_malloc (strlen (dirname) + 1
                                        + strlen (cur_file_name) + 1);
          strcpy (cur_name, dirname);
          strcat (cur_name, "/");
          strcat (cur_name, cur_file_name);
          /* check if older than current */
          if (g_stat (cur_name, &st) == 0
              && S_ISREG (st.st_mode)
              && (! oldest_name || st.st_mtime < oldest_mtime))
            {
              if (oldest_name)
                g_free (oldest_name);
              oldest_name = cur_name;
              oldest_mtime = st.st_mtime;
            }
          else
            g_free (cur_name);
        }
      g_dir_close (dir);
    }

  return oldest_name;
}

static gchar *read_oldest_file_from_dir_and_delete (gchar *dirname)
{ 
  gchar *name, *contents;
  gsize size;
  if ((name = get_oldest_filename_in_dir (dirname)))
    {
      contents = NULL;
      size = 0;
      if (g_file_get_contents (name, &contents, &size, NULL))
        {
          g_unlink (name);
          g_free (name);
          return contents;
        }
      g_free (name);
    }
  return NULL;
}

/* fetch text to display */
static gboolean b_text_fetch_text (BText *text)
{
  gchar *str;
  gsize size;

  /* free old text */
  g_free (text->string);

  /* try to get text from directory */
  if (text->setting_dir[0] != 0)
    {
      if ((str = read_oldest_file_from_dir_and_delete (text->setting_dir)))
        {
          text->string = str;
          return TRUE;
        }
    }

  /* try to get text from file */
  if (text->setting_file[0] != 0)
    {
      if (g_file_get_contents (text->setting_file, &str, &size, NULL))
        {
          text->string = str;
          return TRUE;
        }
    }

  /* try to get text from string */
  if (text->setting_string[0] != 0)
    {
      /* create a copy to work with */
      text->string = g_strdup (text->setting_string);
      return TRUE;
    }

  /* no text available -> fail */
  text->string = g_strdup ("");
  return FALSE;
}

static gboolean
b_text_prepare (BModule  *module,
                GError  **error)
{
  BText *text = B_TEXT (module);
  gboolean text_avail;

  /* correct colors */
  if (text->setting_bg_color > module->maxval)
     text->setting_bg_color = module->maxval;
  if (text->setting_fg_color > module->maxval)
     text->setting_fg_color = module->maxval;

  text->p_font = pChFontNone;
  /* select font chosen by user */
  if (text->setting_font_width >= 1 && text->setting_font_height >= 1
   && text->setting_font_columns >= 1 && text->setting_font_lines >= 1)
    text->p_font = selectChFont (text->setting_font_columns,
                                 text->setting_font_lines,
                                 text->setting_font_width,
                                 text->setting_font_height);
  /* select font that can at least display 3 characters on the display */
  if (text->p_font == pChFontNone)
    text->p_font = selectChFont (3, 1, module->width, module->height);

  /* fetch string to display */
  text_avail = b_text_fetch_text (text);

  /* only proceed if text is available */
  return text_avail ? TRUE : FALSE;
}

static void
b_text_relax (BModule *module)
{
}

static void
b_text_start (BModule *module)
{
  BText *text = B_TEXT (module);

  text->cursor_pos = text->string;
  text->newline    = FALSE;

  text->cursor_x   = 0;
  text->cursor_y   = 0;

  text->anim       = CURSOR_BLINK;
  text->anim_steps = text->setting_init_blink_steps * 2;

  text->exit_blink = FALSE;

  text->timeout    = text->setting_blink_timeout;

  b_module_fill (module, text->setting_bg_color);

  b_module_ticker_start (module, text->timeout);
}

static void
b_text_stop (BModule *module)
{
  BText *text = B_TEXT (module);

  text->player_device_id = -1;
}

static void
b_text_event (BModule      *module,
              BModuleEvent *event)
{
  BText *text = B_TEXT (module);

  switch (event->type)
    {
    case B_EVENT_TYPE_KEY:
      if (text->anim_steps)
        return;

      switch (event->key)
        {
        default:
          break;
        }
      break;

    case B_EVENT_TYPE_PLAYER_ENTERED:
      if (text->player_device_id == -1)
        {
          text->player_device_id = event->device_id;

          module->num_players++;
        }
      break;

    case B_EVENT_TYPE_PLAYER_LEFT:
      if (text->player_device_id == event->device_id)
        {
          text->player_device_id = -1;

          module->num_players--;
        }
      break;

    default:
      break;
    }
}

static gboolean
b_text_tick_intern (BModule *module)
{
  BText *text = B_TEXT (module);
  const ChFont *p_font = text->p_font;
  gboolean cursor_blinked = FALSE;

  /* animation in progress */
  if (text->anim_steps > 0)
    {
      text->anim_steps--;

      switch (text->anim)
        {

          case SCROLL_UP:
            g_memmove (module->buffer,
                       module->buffer + module->width * module->channels,
                       module->width * (module->height - 1) * module->channels);
            b_module_draw_line (module,
                                0, module->height - 1,
                                module->width - 1, module->height - 1,
                                text->setting_bg_color);

            if (text->anim_steps > 0)
              text->timeout = text->setting_scroll_timeout;
              return TRUE;

            /* blink at begin of new line after newline character */
            if (text->newline && text->setting_begin_blink_steps > 0)
              {
                text->anim       = CURSOR_BLINK;
                text->anim_steps = text->setting_begin_blink_steps * 2;
                text->timeout    = text->setting_blink_timeout;
                return TRUE;
              }
            break;

          case CURSOR_BLINK:
            cursor_blinked = TRUE;
            /* draw / clear cursor */
            if (text->cursor_x + p_font->width <= module->width)
              b_module_draw_line (module,
                                  text->cursor_x,
                                  text->cursor_y + p_font->height - 1,
                                  text->cursor_x + p_font->width - 1,
                                  text->cursor_y + p_font->height - 1,
                                  text->anim_steps & 1
                                    ? text->setting_fg_color
                                    : text->setting_bg_color);
            text->timeout = text->setting_blink_timeout;

            if (text->anim_steps > 0)
              return TRUE;
            break;

        } /* switch (text->anim) */

    } /* if (text->anim_steps > 0) */

  /* clear cursor */
  if (text->cursor_x + p_font->width <= module->width)
    b_module_draw_line (module,
                        text->cursor_x,
                        text->cursor_y + p_font->height - 1,
                        text->cursor_x + p_font->width - 1,
                        text->cursor_y + p_font->height - 1,
                        text->setting_bg_color);

  while (1) /* this loop is here to be able to use break and continue */
    {

      /* end of display */
      if (text->cursor_y + p_font->height > module->height)
        {
          /* scroll up one line */
          text->cursor_y  -= p_font->line_advance;
          text->anim       = SCROLL_UP;
          text->anim_steps = p_font->line_advance;
          text->timeout    = text->setting_scroll_timeout;
          break;
        }

      /* end of text */
      if (text->cursor_pos == NULL || *text->cursor_pos == '\0')
        {
            /* stop or blink cursor */
            if (text->setting_exit_blink_steps <= 0 || text->exit_blink)
              {
                b_module_request_stop (module);
                return FALSE;
              }
            text->anim       = CURSOR_BLINK;
            text->anim_steps = text->setting_exit_blink_steps * 2;
            text->timeout    = text->setting_blink_timeout;
            text->exit_blink = TRUE;
            break;
        }

      /* cursor animation at begin of line */
      if (text->newline
          && text->setting_begin_blink_steps > 0
          && ! cursor_blinked)
        {
          text->anim       = CURSOR_BLINK;
          text->anim_steps = text->setting_begin_blink_steps * 2;
          text->timeout    = text->setting_blink_timeout;
          break;
        }

      /* newline character */
      if (*text->cursor_pos == '\n')
        {
          /* cursor animation at end of line */
          if (text->setting_end_blink_steps > 0 && ! cursor_blinked)
            {
              text->anim       = CURSOR_BLINK;
              text->anim_steps = text->setting_end_blink_steps * 2;
              text->timeout    = text->setting_blink_timeout;
              break;
            }
          cursor_blinked = FALSE; /* allow for new cursor blinking at begin of line */

          text->cursor_pos++; /* skip newline character, do not display it */
          text->newline = TRUE;
          /* next line */
          text->cursor_x  = 0;
          text->cursor_y += p_font->line_advance;
          continue;
        }

        text->newline = FALSE;
        cursor_blinked = FALSE;

        /* end of line */
        if (text->cursor_x + p_font->width > module->width)
          {
            /* next line */
            text->cursor_x  = 0;
            text->cursor_y += p_font->line_advance;
            continue;
          }

        /* get character data */
        const gchar *char_data = getChFontChar (p_font, *text->cursor_pos);
        gint x, y;

        /* draw character */
        for (x = 0; x < p_font->width; x++)
          for (y = 0; y < p_font->height; y++)
            {
              gint color = char_data[y * p_font->width + x] == '1'
                             ? text->setting_fg_color
                             : text->setting_bg_color;
              b_module_draw_point (module,
                                   text->cursor_x + x,
                                   text->cursor_y + y,
                                   color);
            }

      /* next character */
      text->cursor_x += p_font->advance;
      text->cursor_pos++;

      /* draw cursor */
      if (text->cursor_x + p_font->width <= module->width)
        b_module_draw_line (module,
                            text->cursor_x,
                            text->cursor_y + p_font->height - 1,
                            text->cursor_x + p_font->width - 1,
                            text->cursor_y + p_font->height - 1,
                            text->setting_fg_color);

      /* wait between characters */
      text->timeout = text->setting_text_timeout;
      break;

    } /* this loop is here to be able to use break and continue */

  return TRUE;
}

static gint
b_text_tick (BModule *module)
{
  BText *text = B_TEXT (module);

  do
    {
      if (! b_text_tick_intern (module))
        return 0;
    }
  while (text->timeout <= 0);

  b_module_paint (module);

  return text->timeout;
}

static void
b_text_describe (BModule      *module,
                 const gchar **title,
                 const gchar **description,
                 const gchar **author)
{
  *title       = "BText";
  *description = "Text display";
  *author      = "Michael Natterer, Stefan Schuermans";
}
