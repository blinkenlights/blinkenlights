/*
 * Created on Sep 21, 2008
 *
 * This code belongs to SQL Power Group Inc.
 */
package de.blinkenlights.bmix.statistics.gui;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;

import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.Icon;
import javax.swing.JLabel;
import javax.swing.JPanel;

public class RoundedLabel extends JPanel {
    private static final int BORDER_WIDTH = 5;
    private final int padding;
    private final JLabel iconLabel;
    private final JLabel textLabel;
	private float iconScale = 1f;
	private boolean greyedOut = false;

    public RoundedLabel(String text, int padding) {
        super();
        setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
        this.padding = padding;
        setOpaque(false);
        setBackground(new Color(0xcc333333, true));
        setForeground(Color.WHITE);
        setBorder(BorderFactory.createEmptyBorder(
                BORDER_WIDTH + padding, BORDER_WIDTH + padding,
                BORDER_WIDTH + padding, BORDER_WIDTH + padding));
        setDoubleBuffered(false);

        iconLabel = new JLabel();
        iconLabel.setAlignmentX(.5f);
        iconLabel.setBorder(BorderFactory.createEmptyBorder(1, 1, 1, 1));
        textLabel = new JLabel(text);
        textLabel.setFont(getFont().deriveFont(8f));
        textLabel.setAlignmentX(.5f);
        textLabel.setForeground(Color.WHITE);
        add(iconLabel);
        add(textLabel);
    }

    public RoundedLabel(int padding) {
        this(null, padding);
    }

    @Override
    public void paintComponent(Graphics gg) {
    	Graphics2D g = (Graphics2D) gg;
        g.setColor(getBackground());
        g.fillRoundRect(
                padding, padding, getWidth() - padding, getHeight() - padding,
                BORDER_WIDTH * 3, BORDER_WIDTH * 3);
        getLayout().layoutContainer(this);
        
        g.translate(iconLabel.getX(), iconLabel.getY());
        iconLabel.paint(g);
        g.translate(-iconLabel.getX(), -iconLabel.getY());
        
        g.translate(textLabel.getX(), textLabel.getY());
        textLabel.paint(g);
        g.translate(-textLabel.getX(), -textLabel.getY());
        
        if(greyedOut) {
            g.setColor(new Color(0, 0, 0, 192));
            g.fillRoundRect(
                    padding, padding, getWidth() - padding, getHeight() - padding,
                    BORDER_WIDTH * 3, BORDER_WIDTH * 3);        	
        }
    }
    

    public void setText(String html) {
        textLabel.setText(html);
    }

    public void setIcon(Icon icon) {
    	if(icon == null) {
    		iconLabel.setIcon(null);
    	}
    	else {
    		iconLabel.setIcon(new ScaledIcon(icon, iconScale));
    	}
    }

	public void setIconScale(float f) {
		iconScale  = f;
	}
	
	public void setGreyedOut(boolean disable) {
		this.greyedOut = disable;
	}
}
