/* blib - Library of useful things to hack the Blinkenlights
 * 
 * Copyright (C) 2002  The Blinkenlights Crew
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

#ifndef __B_PARAMS_H__
#define __B_PARAMS_H__

#define B_TYPE_PARAM_FILENAME           (b_param_filename_get_type ())
#define B_IS_PARAM_SPEC_FILENAME(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), B_TYPE_PARAM_FILENAME))

GType        b_param_filename_get_type  (void) G_GNUC_CONST;
GParamSpec * b_param_spec_filename      (const gchar *name,
                                         const gchar *nick,
                                         const gchar *blurb,
                                         gchar       *default_value,
                                         GParamFlags  flags);

#endif  /* __B_PARAMS_H__ */
