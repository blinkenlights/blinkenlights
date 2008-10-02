/* 
 * This file is part of BMix.
 *
 *    BMix is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 * 
 *    BMix is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with BMix.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */
package de.blinkenlights.bmix.statistics.gui;

import java.awt.Component;
import java.awt.Graphics;
import java.awt.Graphics2D;

import javax.swing.Icon;

public class ScaledIcon implements Icon {
	private final Icon icon;
	private final float scale;

	public ScaledIcon(Icon icon, float scale) {
		this.icon = icon;
		this.scale = scale;
	}

	public int getIconHeight() {
		return (int) (icon.getIconHeight() * scale);
	}

	public int getIconWidth() {
		return (int) (icon.getIconWidth() * scale);
	}

	public void paintIcon(Component c, Graphics gg, int x, int y) {
		Graphics2D g = (Graphics2D) gg.create();
		g.scale(scale, scale);
		icon.paintIcon(c, g, x, y);
	}
}
