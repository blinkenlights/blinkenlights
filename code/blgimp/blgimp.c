/* blgimp - GIMP BlinkenLights Movie save plug-in.
 *
 * Copyright (c) 2001-2002  Sven Neumann <sven@gimp.org>
 *
 * Partly based on code taken from the GIF saving file filter 
 * for The GIMP written by Adam D. Moss <adam@gimp.org>
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
#include <ctype.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>


#define MIN_DURATION    20
#define MAX_DURATION 10000
#define SCALE_WIDTH    100

/* these are only defaults */
#define WIDTH  18
#define HEIGHT  8


typedef struct
{
  gint threshold;
  gint default_duration;
} BLSaveVals;

typedef struct
{
  gint run;
} BLSaveInterface;


static void     query        (void);
static void     run          (gchar      *name,
                              gint        nparams,
                              GimpParam  *param,
                              gint       *nreturn_vals,
                              GimpParam **return_vals);

static gboolean save_image   (gchar      *filename,
                              gint32      image_ID,
                              gint32      drawable_ID,
                              gint32      orig_image_ID);
static gint32   load_image   (gchar      *filename);

static gchar *  bl_fgets     (gchar      *s, 
                              gint        size, 
                              FILE       *stream);
static gint     parse_ms_tag (const gchar *str);

static gint     save_dialog  (gint32       image_ID);


GimpPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

static BLSaveVals blvals =
{
  127,   /* default threshold               */
  100    /* default duration between frames */
};

static BLSaveInterface blint =
{
  FALSE  /* run */
};

MAIN ()

static void
query (void)
{
  static GimpParamDef save_args[] =
  {
    { GIMP_PDB_INT32,    "run_mode",        "Interactive, non-interactive" },
    { GIMP_PDB_IMAGE,    "image",           "Image to save" },
    { GIMP_PDB_DRAWABLE, "drawable",        "Drawable to save" },
    { GIMP_PDB_STRING,   "filename",        "The name of the file to save the image in" },
    { GIMP_PDB_STRING,   "raw_filename",    "The name entered" },
    { GIMP_PDB_INT32,    "default_duration",   "Default duration between framese in milliseconds" }
  };
  static gint nsave_args = sizeof (save_args) / sizeof (save_args[0]);

  static GimpParamDef load_args[] =
  {
    { GIMP_PDB_INT32,    "run_mode",        "Interactive, non-interactive" },
    { GIMP_PDB_STRING,   "filename",        "The name of the file to save the image in" },
    { GIMP_PDB_STRING,   "raw_filename",    "The name entered" },
  };
  static gint nload_args = sizeof (load_args) / sizeof (load_args[0]);

  static GimpParamDef load_return_vals[] =
  {
    { GIMP_PDB_IMAGE, "image", "Output image" }
  };
  static gint nload_return_vals = (sizeof (load_return_vals) /
				   sizeof (load_return_vals[0]));
  
  gimp_install_procedure ("file_blinkenlights_save",
                          "Saves files in BlinkenLights Movie format.",
                          "Saves files in BlinkenLights Movie format.",
                          "Sven Neumann <sven@gimp.org>",
                          "Sven Neumann <sven@gimp.org>",
                          "2001-2002",
                          "<Save>/Blinkenlights",
			  "GRAY*",
                          GIMP_PLUGIN,
                          nsave_args, 0,
                          save_args, NULL);

  gimp_register_save_handler ("file_blinkenlights_save",
			      "blm,blink,blinkenlights",
			      "");

  gimp_install_procedure ("file_blinkenlights_load",
                          "Loads files in BlinkenLights Movie format.",
                          "Loads files in BlinkenLights Movie format.",
                          "Sven Neumann <sven@gimp.org>",
                          "Sven Neumann <sven@gimp.org>",
                          "2001",
                          "<Load>/Blinkenlights",
			  NULL,
                          GIMP_PLUGIN,
                          nload_args, nload_return_vals,
                          load_args, load_return_vals);

  gimp_register_magic_load_handler ("file_blinkenlights_load",
				    "blm,blink,blinkenlights",
				    "",
				    "0,string,# BlinkenLights Movie");
}

static void
run (gchar      *name,
     gint        nparams,
     GimpParam  *param,
     gint       *nreturn_vals,
     GimpParam **return_vals)
{
  static GimpParam     values[2];
  GimpRunModeType      run_mode;
  GimpPDBStatusType    status = GIMP_PDB_SUCCESS;
  gint32               image_ID;
  gint32               drawable_ID;
  gint32               orig_image_ID;
  GimpExportReturnType export = GIMP_EXPORT_CANCEL;

  run_mode = param[0].data.d_int32;

  *nreturn_vals = 1;
  *return_vals  = values;
  values[0].type          = GIMP_PDB_STATUS;
  values[0].data.d_status = GIMP_PDB_EXECUTION_ERROR;

  if (strcmp (name, "file_blinkenlights_save") == 0)
    {
      gimp_ui_init ("blinkenlights", FALSE);

      image_ID    = orig_image_ID = param[1].data.d_int32;
      drawable_ID = param[2].data.d_int32;

      /*  eventually export the image */ 
      switch (run_mode)
	{
	case GIMP_RUN_INTERACTIVE:
	case GIMP_RUN_WITH_LAST_VALS:
	  export = gimp_export_image (&image_ID, &drawable_ID, 
                                      "BlinkenLights", 
				      (GIMP_EXPORT_CAN_HANDLE_GRAY  | 
                                       GIMP_EXPORT_CAN_HANDLE_ALPHA | 
				       GIMP_EXPORT_CAN_HANDLE_LAYERS_AS_ANIMATION));
	  if (export == GIMP_EXPORT_CANCEL)
	    {
	      values[0].data.d_status = GIMP_PDB_CANCEL;
	      return;
	    }
	  break;
	default:
	  break;
	}

      switch (run_mode)
        {
        case GIMP_RUN_INTERACTIVE:
          /*  Possibly retrieve data  */
          gimp_get_data ("file_blinkenlights_save", &blvals);
          
          /*  First acquire information with a dialog  */
          if (! save_dialog (image_ID))
            status = GIMP_PDB_CANCEL;
          break;
          
        case GIMP_RUN_NONINTERACTIVE:
          /*  Make sure all the arguments are there!  */
          if (nparams != 7)
            status = GIMP_PDB_CALLING_ERROR;
          else
            {
              blvals.threshold        = param[5].data.d_int32;
              blvals.default_duration = param[6].data.d_int32;
            }
          break;
	      
        case GIMP_RUN_WITH_LAST_VALS:
          /*  Possibly retrieve data  */
          gimp_get_data ("file_blinkenlights_save", &blvals);
          break;
	      
        default:
          break;
        }

      if (status == GIMP_PDB_SUCCESS)
        {
          if (save_image (param[3].data.d_string,
                          image_ID, drawable_ID, orig_image_ID))
            {
               gimp_set_data ("file_blinkenlights_save", 
                             &blvals, sizeof (BLSaveVals));
            }
          else
            {
              status = GIMP_PDB_EXECUTION_ERROR;
            }
        }

      if (export == GIMP_EXPORT_EXPORT)
        gimp_image_delete (image_ID);
    }
  else if (strcmp (name, "file_blinkenlights_load") == 0)
    {
      image_ID = load_image (param[1].data.d_string);

      if (image_ID != -1)
        {
	  *nreturn_vals = 2;
          values[1].type         = GIMP_PDB_IMAGE;
          values[1].data.d_image = image_ID;
        }
      else
        {
          status = GIMP_PDB_EXECUTION_ERROR;
        }
    }
  else
    {
      status = GIMP_PDB_CALLING_ERROR;
    }    

  values[0].data.d_status = status;
}

static gboolean
save_image (gchar  *filename,
	    gint32  image_ID,
	    gint32  drawable_ID,
	    gint32  orig_image_ID)
{
  GimpPixelRgn   pixel_rgn;
  GimpDrawable  *drawable;
  FILE          *outfile;
  gint           x, y;
  gint           i, n;
  gint           width, height;
  gint           offset_x, offset_y;
  gint           duration;
  guchar        *pixels;
  guchar        *dest;
  gint32        *layers;   
  gint           nlayers;
  gchar         *layer_name;

  
  /* get a list of layers for this image_ID */
  layers = gimp_image_get_layers (image_ID, &nlayers);

  if (nlayers < 1)
    return FALSE;

  /* open the destination file for writing */
  outfile = fopen (filename, "w");
  if (!outfile)
    {
      g_message ("BlinkenLights: Can't open '%s'.", filename);
      return FALSE;
    }

  width  = gimp_image_width (image_ID);
  height = gimp_image_height (image_ID);
  pixels = g_new0 (guchar, width * height * 2);

  /* write the header */
  fprintf (outfile, "# BlinkenLights Movie %dx%d\n", width, height); 

  /* now for each layer in the image, save a frame */

  for (i = nlayers - 1; i >= 0; i--)
    {
      layer_name = gimp_layer_get_name (layers[i]);
      duration = parse_ms_tag (layer_name);
      g_free (layer_name);
 
      if (duration < MIN_DURATION)
        duration = blvals.default_duration;

      fprintf (outfile, "\n@%d\n", duration);

      gimp_drawable_offsets (layers[i], &offset_x, &offset_y);

      drawable = gimp_drawable_get (layers[i]);
      
      gimp_pixel_rgn_init (&pixel_rgn, drawable, 0, 0,
			   drawable->width, drawable->height, FALSE, FALSE);

      for (y = 0; y < height; y++)
        {
          if (offset_x < width                 &&
              offset_x + drawable->width > 0   &&
              y >= offset_y                    &&
              y < offset_y + drawable->height)
            {
              x = MAX (0, offset_x);
              dest = pixels + drawable->bpp * (x + y * width);
              
              gimp_pixel_rgn_get_row (&pixel_rgn, dest,
                                      x - offset_x, y - offset_y,
                                      MIN (drawable->width+offset_x, width-x));
            }
          
          if (drawable->bpp == 1)  /* propagate to 2 channels */
            {
              for (n = width * height; n > 0; n--)
                {
                  pixels[2*(n-1)]   = pixels[n-1];
                  pixels[2*(n-1)+1] = 255;
                }
            }

          for (x = 0; x < width; x++)
            {
              fputc ((pixels[2 * (x + y * width)] > blvals.threshold && 
                      pixels[2 * (x + y * width) + 1]) ? '1' : '0', 
                     outfile);
            }
              
          fputc ('\n', outfile);
        }

      gimp_drawable_detach (drawable);
    }
  
  g_free (pixels);
  g_free (layers);

  fclose (outfile);

  return TRUE;
}

static gint32
load_image (gchar *filename)
{
  gint32  image_ID;
  gint32  layer_ID;
  FILE   *file;
  gchar   buf[1024];
  guchar *data = NULL;
  gint    width;
  gint    height;
  gint    frame = 0;
  gint    line = -1;
  gint    duration;
  gint    len;
  gint    i;
  
  file = fopen (filename, "rb");
  if (!file)
    {
      g_message ("BlinkenLights: Can't open '%s'.", filename);
      return -1;
    }

  if (!bl_fgets (buf, sizeof (buf), file))
    goto error;

  if (buf[0] != '#')
    goto error;

  i = 1;
  while (isspace (buf[i]))
    i++;

  if (g_strncasecmp (buf + i, "BlinkenLights Movie", 19) != 0)
    goto error;

  if (sscanf (buf + i + 19, "%dx%d", &width, &height) != 2)
    {
      g_message ("Blinkenlights files should declare width and height\n"
                 "in the first line, this one doesn't.\n"
                 "I'll assume %d x %d and try to continue.\n", WIDTH, HEIGHT);
      width  = WIDTH;
      height = HEIGHT;
    }

  data = g_new (guchar, 2 * width * height);
  memset (data, 255, 2 * width * height);

  image_ID = gimp_image_new (width, height, GIMP_GRAY);
  gimp_image_set_filename (image_ID, filename);

  while (bl_fgets (buf, sizeof (buf), file))
    {
      len = strlen (buf);

      if (len == 0 || buf[0] == '#')
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
          if (buf[0] == '@' || len - 1 < width)
            {
              g_message ("BlinkenLights: Invalid frame, skipping.");
              line = -1;
            }
          else
            {
              for (i = 0; i < width; i++)
                data[2 * (width * line + i)] = (buf[i] == '1' ? 255 : 0);
              if (++line == height)
                {
                  GimpDrawable *drawable;
                  GimpPixelRgn  pixel_rgn;
                  gchar        *frame_name;

                  frame_name = g_strdup_printf ("Frame %d (%dms)", 
                                                frame++, duration);
                  layer_ID = gimp_layer_new (image_ID, frame_name,
                                             width, height,
                                             GIMP_GRAYA_IMAGE, 100, 
                                             GIMP_NORMAL_MODE);
                  g_free (frame_name);

                  gimp_image_add_layer (image_ID, layer_ID, 0);

                  drawable = gimp_drawable_get (layer_ID);

                  gimp_pixel_rgn_init (&pixel_rgn, drawable, 
                                       0, 0, width, height, TRUE, FALSE);
                  gimp_pixel_rgn_set_rect (&pixel_rgn, data, 
                                           0, 0, width, height);

                  gimp_drawable_flush (drawable);
                  gimp_drawable_detach (drawable);

                  line = -1;
                }
            }            
        }
    }
  
  g_free (data);
  fclose (file);
  return image_ID;

 error:
  g_message ("BlinkenLights: Error parsing '%s'.", filename);
  if (data)
    g_free (data);
  fclose (file);
  return -1;
}


/**** helper functionss ****/

static gint
parse_ms_tag (const gchar *str)
{
  gint sum    = 0;
  gint offset = 0;
  gint length;

  length = strlen (str);

find_another_bra:
  
  while ((offset<length) && (str[offset]!='('))
    offset++;
  
  if (offset>=length)
    return(-1);

  if (!isdigit(str[++offset]))
    goto find_another_bra;

  do
    {
      sum *= 10;
      sum += str[offset] - '0';
      offset++;
    }
  while ((offset<length) && (isdigit(str[offset])));  

  if (length-offset <= 2)
    return(-3);

  if ((toupper(str[offset]) != 'M') || (toupper(str[offset+1]) != 'S'))
    return(-4);

  return (sum);
}

/* die Macintosh die */
gchar *
bl_fgets (gchar *s, 
          gint   size, 
          FILE  *stream)
{
  gint i = 0;
  gint c = 0;

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


/**** GUI functions ****/

static void
save_ok_callback (GtkWidget *widget,
		  gpointer   data)
{
  blint.run = TRUE;

  gtk_widget_destroy (GTK_WIDGET (data));
}

static gint
save_dialog (gint32 image_ID)
{
  GtkWidget *dlg;
  GtkObject *adj;
  GtkWidget *frame;
  GtkWidget *label;
  GtkWidget *vbox;
  GtkWidget *table;

  dlg = gimp_dialog_new ("Save as BlinkenLights Movie", "blinkenlights",
			 gimp_standard_help_func, NULL,
			 GTK_WIN_POS_MOUSE,
			 FALSE, TRUE, FALSE,

			 "OK", save_ok_callback,
			 NULL, NULL, NULL, TRUE, FALSE,
			 "Cancel", gtk_widget_destroy,
			 NULL, 1, NULL, FALSE, TRUE,

			 NULL);

  gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
		      GTK_SIGNAL_FUNC (gtk_main_quit),
		      NULL);

  vbox = GTK_DIALOG (dlg)->vbox;
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);

  frame = gtk_frame_new ("BlinkenLights Options");
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_IN);
  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  table = gtk_table_new (2, 4, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_container_set_border_width (GTK_CONTAINER (table), 4);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  adj = gimp_scale_entry_new (GTK_TABLE (table), 0, 0,
			      "Value Threshold:", SCALE_WIDTH, 0,
			      blvals.threshold, 
                              0, 255, 1, 8, 0,
			      TRUE, 0, 0,
			      NULL, NULL);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      GTK_SIGNAL_FUNC (gimp_int_adjustment_update),
		      &blvals.threshold);

  adj = gimp_scale_entry_new (GTK_TABLE (table), 0, 1,
			      "Default Duration:", SCALE_WIDTH, 0,
			      blvals.default_duration, 
                              MIN_DURATION, MAX_DURATION, 10, 100, 0,
			      TRUE, 0, 0,
			      NULL, NULL);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      GTK_SIGNAL_FUNC (gimp_int_adjustment_update),
		      &blvals.default_duration);

  label = gtk_label_new ("Milliseconds");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 3, 4, 1, 2);
  gtk_widget_show (label);

  gtk_widget_show (dlg);

  gtk_main ();
  gdk_flush ();

  return blint.run;
}
