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

    public RoundedLabel(String text) {
        super(text);
        setOpaque(false);
        setBackground(new Color(0xcc333333, true));
        setForeground(Color.WHITE);
        setBorder(BorderFactory.createEmptyBorder(BORDER_WIDTH, BORDER_WIDTH, BORDER_WIDTH, BORDER_WIDTH));
        setFont(getFont().deriveFont(8f));
    }

    public RoundedLabel() {
        this(null);
    }

    @Override
    public void paint(Graphics g) {
        g.setColor(getBackground());
        g.fillRoundRect(0, 0, getWidth(), getHeight(), BORDER_WIDTH * 3, BORDER_WIDTH * 3);
        super.paint(g);
    }
}
