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

#ifndef __B_EFFECTS_H__
#define __B_EFFECTS_H__

G_BEGIN_DECLS

typedef enum
{
  B_EFFECT_SCOPE_NONE  = 0,
  B_EFFECT_SCOPE_LEFT  = 1 << 0,
  B_EFFECT_SCOPE_RIGHT = 1 << 1,
  B_EFFECT_SCOPE_ALL   = B_EFFECT_SCOPE_LEFT | B_EFFECT_SCOPE_RIGHT
} BEffectScope;

#define B_TYPE_EFFECTS            (b_effects_get_type ())
#define B_EFFECTS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_EFFECTS, BEffects))
#define B_EFFECTS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_EFFECTS, BEffectsClass))
#define B_IS_EFFECTS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_EFFECTS))
#define B_IS_EFFECTS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), B_TYPE_EFFECTS))

typedef struct _BEffectsClass  BEffectsClass;

struct _BEffectsClass
{
  GObjectClass  parent_class;
};

struct _BEffects
{
  GObject       parent_instance;

  BEffectScope  invert;
  BEffectScope  vflip;
  BEffectScope  hflip;
  BEffectScope  lmirror;
  BEffectScope  rmirror;
};

GType      b_effects_get_type (void) G_GNUC_CONST;
BEffects * b_effects_new      (void);
void       b_effects_apply    (BEffects *effects,
                               guchar   *frame_data,
                               gint      width,
                               gint      height,
                               gint      channels,
                               gint      maxval);

G_END_DECLS

#endif /* __B_EFFECTS_H__ */
