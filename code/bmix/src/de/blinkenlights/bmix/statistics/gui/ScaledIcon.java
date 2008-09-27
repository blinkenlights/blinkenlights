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
