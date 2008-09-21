/*
 * Created on Sep 21, 2008
 *
 * This code belongs to SQL Power Group Inc.
 */
package de.blinkenlights.bmix.statistics.gui;

import java.awt.Color;
import java.awt.Graphics;

import javax.swing.BorderFactory;
import javax.swing.JLabel;

public class RoundedLabel extends JLabel {

    private static final int BORDER_WIDTH = 5;
    private final int padding;

    public RoundedLabel(String text, int padding) {
        super(text);
        this.padding = padding;
        setOpaque(false);
        setBackground(new Color(0xcc333333, true));
        setForeground(Color.WHITE);
        setBorder(BorderFactory.createEmptyBorder(
                BORDER_WIDTH + padding, BORDER_WIDTH + padding,
                BORDER_WIDTH + padding, BORDER_WIDTH + padding));
        setFont(getFont().deriveFont(8f));
    }

    public RoundedLabel(int padding) {
        this(null, padding);
    }

    @Override
    public void paint(Graphics g) {
        g.setColor(getBackground());
        g.fillRoundRect(
                padding, padding, getWidth() - padding, getHeight() - padding,
                BORDER_WIDTH * 3, BORDER_WIDTH * 3);
        super.paint(g);
    }
}
