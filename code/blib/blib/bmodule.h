/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2002  The Blinkenlights Crew
 *                          Daniel Mack <daniel@yoobay.net>
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

#ifndef __B_MODULE_H__
#define __B_MODULE_H__

G_BEGIN_DECLS

struct _BModuleEvent
{
  gint             device_id;
  BModuleEventType type;
  BModuleKey       key;
};

typedef gboolean (* BModulePaintCallback) (BModule  *bmodule,
                                           guchar   *buffer,
                                           gpointer  data);


#define B_TYPE_MODULE            (b_module_get_type ())
#define B_MODULE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_MODULE, BModule))
#define B_MODULE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_MODULE, BModuleClass))
#define B_IS_MODULE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_MODULE))
#define B_IS_MODULE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), B_TYPE_MODULE))
#define B_MODULE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), B_TYPE_MODULE, BModuleClass))

typedef struct _BModuleClass BModuleClass;

struct _BModule
{
  GObject               parent_instance;

  gint                  width;
  gint                  height;
  gint                  channels;
  gint                  maxval;
  gdouble               aspect;

  gdouble               speed;
  gint                  lifetime;

  gint                  num_players;

  guchar               *buffer;

  /*< private >*/
  gboolean              owns_buffer;

  BModulePaintCallback  paint_callback;
  gpointer              paint_data;

  gboolean              ready;    /* TRUE between prepare() and relax() */
  gboolean              running;  /* TRUE between start() and stop()    */
  guint                 tick_source_id;
  guint                 life_source_id;

  gpointer              pad1;
  gpointer              pad2;
  gpointer              pad3;
  gpointer              pad4;
};

struct _BModuleClass
{
  GObjectClass  parent_class;

  /* max_players defaults to 0. It needs to be set to a value between
     1 and 4 to indicate that the module can handle events from this
     number of different devices. */
  gint     max_players;

  /* query() can be called w/o a module instance, it returns TRUE if
     the module class can handle the given parameters, FALSE otherwise. */
  gboolean (* query)    (gint          width,
                         gint          height,
                         gint          channels,
                         gint          maxval);
  
  /* In prepare() the module prepares itself for being started but it
     doesn't perform any visible action yet, i.e. it may not call
     b_module_paint().                                                  */
  gboolean (* prepare)  (BModule      *module,
                         GError      **error);
  /* In relax() the module releases any resources it has allocated in
     the prepare() method.                                              */
  void     (* relax)    (BModule      *module);

  /* start() signals that the module should start to run. It may call
     b_module_paint() to clear the screen or show an intro and it
     should call b_module_start_ticker() if it needs ticks.             */
  void     (* start)    (BModule      *module);

  /* stop() signals that the module is stopped. It may call
     b_module_paint() once more (but it shouldn't).                     */
  void     (* stop)     (BModule      *module);

  /* An event occured and the module may do anything in response.       */
  void     (* event)    (BModule      *bmodule,
                         BModuleEvent *event);

  /* A previously set timeout expired. The module should do whatever it
     likes and return a new timeout (in milliseconds). If the return
     value is <= 0, no new timeout is installed.                        */
  gint     (* tick)     (BModule      *module);

  void     (* describe) (BModule      *module,
                         const gchar **title,
                         const gchar **description,
                         const gchar **author); 

  /* padding for future use */
  gpointer  pad1;
  gpointer  pad2;
  gpointer  pad3;
  gpointer  pad4;
};


GType     b_module_get_type     (void) G_GNUC_CONST;


/* functions the module implementations may call */

void      b_module_ticker_start (BModule *module,
                                 gint     timeout);
void      b_module_ticker_stop  (BModule *module);
void      b_module_request_stop (BModule *module);
void      b_module_paint        (BModule *module);

G_END_DECLS

#endif /* __B_MODULE_H__ */
