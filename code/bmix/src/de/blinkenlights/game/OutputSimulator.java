/*
 * Created on Sep 25, 2008
 *
 * This code belongs to SQL Power Group Inc.
 */
package de.blinkenlights.game;

import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.RenderingHints;
import java.awt.image.BufferedImage;

import javax.swing.JFrame;
import javax.swing.JPanel;

public class OutputSimulator implements FrameTarget {

    private JFrame frame;
    private Image image;
    
    public synchronized void putFrame(BufferedImage image) {
        this.image = image;
        frame.repaint();
    }

    public void start() {
        frame = new JFrame("Output!");
        frame.setLocationByPlatform(true);
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        
        JPanel imagePane = new JPanel() {
            @Override
            protected void paintComponent(Graphics g) {
                Graphics2D g2 = (Graphics2D) g;
                g2.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR);
                super.paintComponent(g);
                synchronized (OutputSimulator.this) {
                    if (image != null) {
                        g.drawImage(image, 0, 0, getWidth(), getHeight(), null);
                    }
                }
            }
        };
        
        frame.setContentPane(imagePane);
        frame.setSize(new Dimension(320,200));
        frame.setVisible(true);
    }

    public synchronized void stop() {
        image = null;
        frame.dispose();
    }

    public void gameEnding() {
//        image = null;
//        frame.repaint();
    }

}
