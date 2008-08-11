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

#ifndef __BL_PONG_H__
#define __BL_PONG_H__

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define BL_TYPE_PONG            (bl_pong_get_type ())
#define BL_PONG(obj)            (GTK_CHECK_CAST ((obj), BL_TYPE_PONG, BlPong))
#define BL_PONG_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), BL_TYPE_PONG, BlPongClass))
#define BL_IS_PONG(obj)         (GTK_CHECK_TYPE ((obj), BL_TYPE_PONG))
#define BL_IS_PONG_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), BL_TYPE_PONG))

typedef enum
{
  RIGHT,
  LEFT
} XDirection;

typedef enum
{
  UP,
  DOWN
} YDirection;


typedef enum
{
  GAME_INIT,
  LPLAYER_JOIN,
  RPLAYER_JOIN,
  GAME_OVER
} AnimType;

typedef struct _BlPongClass  BlPongClass;

struct _BlPongClass
{
  GtkObjectClass  parent_class;

  void (* started)   (BlPong *pong);
  void (* finished)  (BlPong *pong);
  void (* new_frame) (BlPong *pong);
};

struct _BlPong
{
  GtkObject       parent_instance;
  
  BlIsdn         *isdn;

  gint            width;
  gint            height;
  guchar         *matrix;

  guint           timeout_id;

  AnimType        anim;
  gint            anim_steps;

  gint            lpaddle;
  gint            rpaddle;
  gint            ball_x;
  gint            ball_y;
  XDirection      ball_xdir;
  YDirection      ball_ydir;

  gboolean        lplayer_human;
  gboolean        rplayer_human;
};

GtkType    bl_pong_get_type (void);
BlPong   * bl_pong_new      (BlIsdn *isdn,
                             gint    width,
                             gint    height);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BL_PONG_H__ */
