/* bmatrix.c Matrix Module for blib
 *
 * Copyright (c) 2002  Simon Budig  <simon@gimp.org>,
 *               1999  Jamie Zawinski   <jwz@jwz.org>
 *
 * Inspired/Ripped off from the Matrix hack from the xscreensaver package.
 *
 * Due to the very hackish nature of this hack no warranties for the
 * quality of the code are given. Sorry.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 *
 * Matrix -- simulate the text scrolls from the movie "The Matrix".
 *
 */

#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <gmodule.h>

#include <blib/blib.h>


#define TIMEOUT  60
#define MATRIX_DENSITY 40     /* Average density of used pixels           */
#define MATRIX_INSERT  "both" /*  "top" "bottom" "both"                   */
#define MATRIX_ACTION  0      /* special effects - flashing, errors, etc. */
#define MATRIX_SPINNER 0      /* number of stationary flashing pixels     */

/* Uncomment this for 2 pixels wide columns
#define MATRIX_WIDE
 */

#define B_TYPE_MATRIX         (b_type_matrix)
#define B_MATRIX(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_MATRIX, BMatrix))
#define B_MATRIX_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_MATRIX, BMatrixClass))
#define B_IS_MATRIX(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_MATRIX))

typedef struct _BMatrix       BMatrix;
typedef struct _BMatrixClass  BMatrixClass;

typedef struct _BMatrixCell   BMatrixCell;
typedef struct _BMatrixFeeder BMatrixFeeder;


struct _BMatrixCell
{
  guint glyph   : 8;
   gint glow    : 8;
  guint changed : 1;
  guint spinner : 1;
};

struct _BMatrixFeeder
{
  gint remaining;
  gint throttle;
  gint y;
  gint failure;
  gint fail_type;
};

struct _BMatrix
{
  BModule   parent_instance;

  gint grid_width, grid_height;
  gint flash_x, flash_y, flash_count;
  gint failcount;
  BMatrixCell *cells;
  BMatrixFeeder *feeders;
  gint action_p;
  gint insert_top_p, insert_bottom_p;
  gint density;
  gint density_param;
 
  gint image_width, image_height;
  gint nglyphs;
};

struct _BMatrixClass
{
  BModuleClass  parent_class;
};

enum
{
  PROP_0,
  PROP_LINES
};

static gint densitizer (BMatrix *state);
static void init_spinners (BMatrix *state);
static void draw_matrix (BMatrix *state);


static guchar intensities[] =
{   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    3,  4,  5,  6,  6,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
};

static GType    b_matrix_get_type   (GTypeModule   *module);
static void     b_matrix_class_init (BMatrixClass  *klass);
static gboolean b_matrix_query      (gint           width,
                                     gint           height,
                                     gint           channels,
                                     gint           maxval);
static gboolean b_matrix_prepare    (BModule       *module,
                                     GError       **error);
static void     b_matrix_relax      (BModule       *module);
static void     b_matrix_start      (BModule       *module);
static gint     b_matrix_tick       (BModule       *module);
static void     b_matrix_describe   (BModule       *module,
                                     const gchar  **title,
                                     const gchar  **description,
                                     const gchar  **author);

static GType  b_type_matrix = 0;


G_MODULE_EXPORT gboolean
b_module_register (GTypeModule *module)
{
  b_matrix_get_type (module);
  return TRUE;
}

GType
b_matrix_get_type (GTypeModule *module)
{
  if (!b_type_matrix)
    {
      static const GTypeInfo matrix_info =
      {
        sizeof (BMatrixClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_matrix_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BMatrix),
        0,              /* n_preallocs */
        NULL            /* instance_init */
      };

      b_type_matrix = g_type_module_register_type (module,
                                                   B_TYPE_MODULE, "BMatrix",
                                                   &matrix_info, 0);
    }

  return b_type_matrix;
}

static void
b_matrix_class_init (BMatrixClass *klass)
{
  BModuleClass *module_class;

  module_class = B_MODULE_CLASS (klass);

  module_class->query    = b_matrix_query;
  module_class->prepare  = b_matrix_prepare;
  module_class->relax    = b_matrix_relax;
  module_class->start    = b_matrix_start;
  module_class->tick     = b_matrix_tick;
  module_class->describe = b_matrix_describe;
}

static gboolean
b_matrix_query (gint  width,
                gint  height,
                gint  channels,
                gint  maxval)
{
  return (width > 0 && height > 0 && maxval == 255 && channels == 1);
}

static gboolean
b_matrix_prepare (BModule  *module,
                  GError  **error)
{
  BMatrix *state  = B_MATRIX (module);
  gchar   *insert = NULL;

  state->nglyphs = 15;

#ifdef MATRIX_WIDE
  state->grid_width  = (module->width + 1) / 3;
#else
  state->grid_width  = module->width;
#endif
  state->grid_height = module->height;

  state->flash_count = -rand() % 1000;

  state->cells = g_new0 (BMatrixCell, state->grid_width * state->grid_height);
  state->feeders = g_new0 (BMatrixFeeder, state->grid_width);

  state->density = MATRIX_DENSITY;
  if (state->density < 0) state->density = 0;
  if (state->density > 100) state->density = 100;
  state->density_param = densitizer (state);

  insert = MATRIX_INSERT;
  if (insert && !strcmp(insert, "top"))
    {
      state->insert_top_p = TRUE;
      state->insert_bottom_p = FALSE;
    }
  else if (insert && !strcmp(insert, "bottom"))
    {
      state->insert_top_p = FALSE;
      state->insert_bottom_p = TRUE;
    }
  else if (insert && !strcmp(insert, "both"))
    {
      state->insert_top_p = TRUE;
      state->insert_bottom_p = TRUE;
    }
  else
    {
      /* if (insert && *insert)
        fprintf (stderr,
                 "%s: `insert' must be `top', `bottom', or `both', not `%s'\n",
                 "bmatrix", insert); */
      state->insert_top_p = FALSE;
      state->insert_bottom_p = TRUE;
    }

  state->action_p = MATRIX_ACTION;

  init_spinners (state);

  return TRUE;
}

static void
b_matrix_relax (BModule *module)
{
  BMatrix *matrix = B_MATRIX (module);

  if (matrix->cells)
    {
      g_free (matrix->cells);
      matrix->cells = NULL;
    }
  if (matrix->feeders)
    {
      g_free (matrix->feeders);
      matrix->feeders = NULL;
    }
}

static void
b_matrix_start (BModule *module)
{
  b_module_fill (module, 0);
  b_module_ticker_start (module, TIMEOUT);
}

static gint
b_matrix_tick (BModule *module)
{
  BMatrix *matrix = B_MATRIX (module);

  draw_matrix (matrix);
  b_module_paint (module);

  return TIMEOUT;
}

static void
b_matrix_describe (BModule      *module,
                   const gchar **title,
                   const gchar **description,
                   const gchar **author)
{
  *title       = "BMatrix";
  *description = "Matrix hack";
  *author      = "Simon Budig, Jamie Zawinski";
}

static gint
densitizer (BMatrix *state)
{
  /* Horrid kludge that converts percentages (density of screen coverage)
     to the parameter that actually controls this.  I got this mapping
     empirically, on a 1024x768 screen.  Sue me. */
  /* Used to determine a reasonable starting value, xmatrix tries to
     adjust this parameter while running. */
  if      (state->density < 10) return 85;
  else if (state->density < 15) return 60;
  else if (state->density < 20) return 45;
  else if (state->density < 25) return 25;
  else if (state->density < 30) return 20;
  else if (state->density < 35) return 15;
  else if (state->density < 45) return 10;
  else if (state->density < 50) return 8;
  else if (state->density < 55) return 7;
  else if (state->density < 65) return 5;
  else if (state->density < 80) return 3;
  else if (state->density < 90) return 2;
  else return 1;
}


static void
init_spinners (BMatrix *state)
{
  gint i = MATRIX_SPINNER;
  gint x, y;
  BMatrixCell *cell;

  for (y = 0; y < state->grid_height; y++)
    for (x = 0; x < state->grid_width; x++)
      {
        cell = &state->cells[state->grid_width * y + x];
        cell->spinner = 0;
      }

  while (--i > 0)
    {
      x = rand() % state->grid_width;
      y = rand() % state->grid_height;
      cell = &state->cells[state->grid_width * y + x];
      cell->spinner = 1;
    }
}

static void
insert_glyph (BMatrix *state, gint glyph, gint x, gint y)
{
  gint bottom_feeder_p = (y >= 0);
  BMatrixCell *from, *to;

  if (y >= state->grid_height)
    return;

  if (bottom_feeder_p)
    {
      to = &state->cells[state->grid_width * y + x];
    }
  else
    {
      for (y = state->grid_height-1; y > 0; y--)
        {
          from = &state->cells[state->grid_width * (y-1) + x];
          to   = &state->cells[state->grid_width * y     + x];
          to->glyph   = from->glyph;
          to->glow    = from->glow;
          to->changed = 1;
        }
      to = &state->cells[x];
    }

  to->glyph = glyph;
  to->changed = 1;

  if (!to->glyph)
   ;
  else if (bottom_feeder_p)
    to->glow = 1 + (rand() % 2);
  else
    to->glow = 0;
}


static void
feed_matrix (BMatrix *state)
{
  gint x;

  /* Update according to current feeders. */
  for (x = 0; x < state->grid_width; x++)
    {
      BMatrixFeeder *f = &state->feeders[x];

      if (f->throttle)          /* this is a delay tick, synced to frame. */
        {
          f->throttle--;
        }
      else if (f->remaining > 0)        /* how many items are in the pipe */
        {
          gint g = (rand() % state->nglyphs) + 1;
          insert_glyph (state, g, x, f->y);
          f->remaining--;
          if (f->y >= 0)  /* bottom_feeder_p */
            f->y++;
        }
      else                              /* if pipe is empty, insert spaces */
        {
          insert_glyph (state, 0, x, f->y);
          if (f->y >= 0)  /* bottom_feeder_p */
            f->y++;
        }

      if ((rand() % 10) == 0)         /* randly change throttle speed */
        {
          f->throttle = ((rand() % 5) + (rand() % 5));
        }
    }
}


static void
hack_matrix (BMatrix *state)
{
  gint x;

  /* Glow some characters. */
  if (!state->insert_bottom_p)
    {
      gint i = rand() % (state->grid_width / 2);
      while (--i > 0)
        {
          gint x = rand() % state->grid_width;
          gint y = rand() % state->grid_height;
          BMatrixCell *cell = &state->cells[state->grid_width * y + x];
          if (cell->glyph && cell->glow == 0)
            {
              cell->glow = rand() % 10;
              cell->changed = 1;
            }
        }
    }

  /* Change some of the feeders. */

  state->failcount = 0;

  for (x = 0; x < state->grid_width; x++)
    {
      BMatrixFeeder *f = &state->feeders[x];
      gint bottom_feeder_p;

      if (state->action_p)
        /* failure of some columns is controlled by a counter:
           f->failure == 0: normal operation,
           f->failure < -1: flickering, will black out soon,
           f->failure == -1: black out
           f->failure > 1: still black out...
           f->failure == 1: resuming operation
        */
        {
          if (f->failure != 0)
            {
              f->failure += (f->failure < 0 ? 1 : -1);
              state->failcount++;
            }

          if (f->failure < -1)
            {
              gint y;
              gint r = rand() % 10;
              BMatrixCell *cell;
              f->fail_type = r > 2 ? 1 : 2;
              if (f->fail_type == 2)
              for (y=0; y < state->grid_height; y++)
                {
                  cell = &state->cells[state->grid_width * y + x];
                  if (cell->glyph)
                    {
                      cell->glow = 1;
                      cell->changed = 1;
                    }
                }
            }

          if (f->failure == -1)
            {
              f->failure += 150;  /* This should be variable, but equal
                                     for failing neighbours   */
              f->fail_type = 1;
            }

          if (f->failure == 1)
            {
              gint y;
              BMatrixCell *cell;

              f->fail_type = 0;
              for (y=0; y < state->grid_height; y++)
                {
                  cell = &state->cells[state->grid_width * y + x];
                  if (cell->glyph)
                    {
                      cell->glow = 1;
                      cell->changed = 1;
                    }
                }
            }
        }

      if (f->remaining > 0)     /* never change if pipe isn't empty */
        continue;

      /* if ((rand() % densitizer(state)) != 0) then change N% of the time */
      if ((rand() % state->density_param) != 0) /* then change N% of the time */
        continue;

      f->remaining = 3 + (rand() % state->grid_height);
      f->throttle = ((rand() % 5) + (rand() % 5));

      if ((rand() % 4) != 0)
        f->remaining = 0;

      if (state->insert_top_p && state->insert_bottom_p)
        bottom_feeder_p = (rand() & 1);
      else
        bottom_feeder_p = state->insert_bottom_p;

      if (bottom_feeder_p)
        f->y = rand() % (state->grid_height / 2);
      else
        f->y = -1;
    }

  if (state->action_p)
    {
      /* Some columns may fail - they typically infect their neighbours */
      gint failtimeout = (rand() % 12) + 3;

      if (!(rand() % 200) && state->failcount <= state->grid_width / 6)
        {
          gint w = rand() % (1 + state->grid_width / 12);
          gint i;

          x = (rand() % state->grid_width);

          for (i = -w; i <= w; i++)
            {
              if (x+i >= 0 &&
                  x+i < state->grid_width &&
                  !(state->feeders[x+i].failure))
                {
                  state->feeders[x+i].failure =
                    -failtimeout - w + (rand() % (abs(i/3)+1));
                }
            }
        }

      /* Sometimes a flash (expanding square of glowing glyphs) will appear.
        state->flash_count < 0: Count up to next flash
        state->flash_count >= 0: radius of flash
      */
      if (state->flash_count <= state->grid_width + state->grid_height)
        state->flash_count++;
      else
        state->flash_count = - 1000 + rand() % 800;

      if (state->flash_count == 0)
        {
          BMatrixCell *cell;
          state->flash_x = rand() % state->grid_width;
          state->flash_y = rand() % state->grid_height;
          cell =
            &state->cells[state->grid_width * state->flash_y + state->flash_x];
          if (cell->glyph && !(cell->glow > 0))
            {
              cell->glow    = 1;
              cell->changed = 1;
            }
        }

      if (state->flash_count > 0)
        {
          BMatrixCell *cell;
          gint j, k, x, y;
          k = state->flash_count + state->flash_count / 4;
          for (; state->flash_count < k; state->flash_count++)
            {
              for (j=0; j <= 2 * state->flash_count; j += 1)
                {
                  x = state->flash_x - state->flash_count + j;
                  y = state->flash_y - state->flash_count;
                  if (x >= 0 && x < state->grid_width && y >= 0 &&
                      y < state->grid_height)
                    {
                      cell = &state->cells[state->grid_width * y + x];
                      if (cell->glyph) { cell->glow+=2; cell->changed = 1; }
                    }
                  y = state->flash_y + state->flash_count;
                  if (x >= 0 && x < state->grid_width && y >= 0 &&
                      y < state->grid_height)
                    {
                      cell = &state->cells[state->grid_width * y + x];
                      if (cell->glyph) { cell->glow+=2; cell->changed = 1; }
                    }
                  y = state->flash_y - state->flash_count + j;
                  x = state->flash_x - state->flash_count;
                  if (x >= 0 && x < state->grid_width && y >= 0 &&
                      y < state->grid_height)
                    {
                      cell = &state->cells[state->grid_width * y + x];
                      if (cell->glyph) { cell->glow+=2; cell->changed = 1; }
                    }
                  x = state->flash_x + state->flash_count;
                  if (x >= 0 && x < state->grid_width && y >= 0 &&
                      y < state->grid_height)
                    {
                      cell = &state->cells[state->grid_width * y + x];
                      if (cell->glyph) { cell->glow+=2; cell->changed = 1; }
                    }
                }
            }
        }
    }

  if (! (rand() % 500))
    init_spinners (state);
}

static void
draw_matrix (BMatrix *state)
{
  gint x, y, i;
  gint count = 0;
  BModule *module = B_MODULE (state);

# define WINDOW 25
  gint dens;
  static gint ndens = 0;
  static gint tdens = 0;
  static gint *densities = NULL;
  static gint *parameters = NULL;
  static gint dens_sum, param_sum;

  feed_matrix (state);
  hack_matrix (state);

  for (x = 0; x < state->grid_width; x++)
    {
      /* BMatrixFeeder *f = &(state->feeders[x]); */

      for (y = 0; y < state->grid_height; y++)
        {
          BMatrixCell *cell = &state->cells[state->grid_width * y + x];

          if (cell->glyph)
            count++;

          if (!cell->changed)
            continue;

#ifdef MATRIX_WIDE
          if (cell->glyph == 0)
            {
              b_module_draw_point (module, x*3, y, 0);
              b_module_draw_point (module, x*3+1, y, 0);
            }
          else
            {
              b_module_draw_point (module, x*3, y, 17 * intensities[
                                       ((cell->glow > 0 || cell->spinner) ?
                                         cell->glyph + 30
                                       : (cell->glow == 0
                                         ? cell->glyph + 15
                                         : cell->glyph)) -1]);
              b_module_draw_point (module, x*3+1, y, 17 * intensities[
                                       ((cell->glow > 0 || cell->spinner) ?
                                         cell->glyph + 30
                                       : (cell->glow == 0
                                         ? cell->glyph + 15
                                         : cell->glyph)) -1]);
            }
#else
          if (cell->glyph == 0)
            {
              b_module_draw_point (module, x, y, 0);
            }
          else
            {
              b_module_draw_point (module, x, y, 17 * intensities[
                                       ((cell->glow > 0 || cell->spinner) ?
                                         cell->glyph + 30
                                       : (cell->glow == 0
                                         ? cell->glyph + 15
                                         : cell->glyph)) -1]);
            }
#endif

          cell->changed = 0;

          if (cell->glow > 0)
            {
              cell->glow--;
              cell->changed = 1;
            }
          else if (cell->glow < 0)
            {
              cell->glow++;
              if (cell->glow == 0)
                cell->glyph = 0;
              cell->changed = 1;
            }

          if (cell->spinner)
            {
              cell->glyph = rand() % state->nglyphs;
              cell->changed = 1;
            }
        }
    }

  /* Adjust density-parameter */
  dens = (100 * count) / (state->grid_width * state->grid_height);

  if (!densities) {
    densities = g_new (gint, WINDOW);
    for (i = 0; i < WINDOW; i++) densities[i] = state->density;
    dens_sum = state->density * WINDOW;
  }
  if (!parameters) {
    parameters = g_new (gint, WINDOW);
    for (i = 0; i < WINDOW; i++) parameters[i] = state->density_param;
    param_sum = state->density_param * WINDOW;
  }

  /* collect the last WINDOW density/parameter values */

  dens_sum -= densities [ndens % WINDOW];
  param_sum -= parameters [ndens % WINDOW];

  densities [ndens % WINDOW] = dens;
  parameters [ndens % WINDOW] = state->density_param;

  dens_sum += densities [ndens % WINDOW];
  param_sum += parameters [ndens % WINDOW];

  tdens += dens;
  ndens++;

  /* assume, that the average parameter results in the average density
   * and adjust the parameter accordingly +- 1 */

  if (dens_sum <= (state->density - 5) * WINDOW &&
      state->density_param > 1)
    state->density_param = param_sum / WINDOW - 1;

  if (dens_sum >= (state->density + 5) * WINDOW)
    state->density_param = param_sum / WINDOW + 2;
    /* +2 to jump over the rounding error */

#if 0
  printf ("density: %d%% (%d%%), param: %d\n",
          dens, (tdens / ndens), state->density_param);
#endif
}
