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

#ifndef __BL_THEATER_H__
#define __BL_THEATER_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define BL_TYPE_THEATER            (bl_theater_get_type ())
#define BL_THEATER(obj)            (GTK_CHECK_CAST ((obj), BL_TYPE_THEATER, BlTheater))
#define BL_THEATER_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), BL_TYPE_THEATER, BlTheaterClass))
#define BL_IS_THEATER(obj)         (GTK_CHECK_TYPE ((obj), BL_TYPE_THEATER))
#define BL_IS_THEATER_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), BL_TYPE_THEATER))

typedef struct _Packet Packet;
struct _Packet
{
  u_int32_t magic;
  u_int32_t count;
  u_int16_t width;
  u_int16_t height;
  u_int8_t  data;
};


typedef struct _BlTheaterClass  BlTheaterClass;

struct _BlTheaterClass
{
  GtkObjectClass   parent_class;

  void (* movie_started)  (BlTheater *theater,
                           BlTheater *movie);
  void (* movie_finished) (BlTheater *theater,
                           BlTheater *movie);
};

struct _BlTheater
{
  GtkObject        parent_instance;

  gint            *socks;
  gint             n_socks;

  gint             width;
  gint             height;

  Packet          *packet;
  Packet          *empty;
  gint             frame_count;

  guint            timeout_id;

  BlPong          *pong;

  BlMovie         *movie;
  GList           *current;

  BlPreview       *preview;
};


GtkType     bl_theater_get_type      (void);
BlTheater * bl_theater_new           (gint        width,
                                      gint        height,
                                      gint       *socks,
                                      gint        n_socks,
                                      BlPong     *pong);
void        bl_theater_set_playlist  (BlTheater  *theater,
                                      BlPlayList *playlist);
void        bl_theater_set_preview   (BlTheater  *theater,
                                      BlPreview  *preview);
void        bl_theater_set_movie     (BlTheater  *theater,
                                      BlMovie    *movie);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BL_THEATER_H__ */
