/*
 * Created on Sep 25, 2008
 *
 * This code belongs to SQL Power Group Inc.
 */
package de.blinkenlights.game;

import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Image;

import javax.swing.JFrame;
import javax.swing.JPanel;

public class OutputSimulator implements FrameTarget {

    private JFrame frame;
    private Image image;
    
    public synchronized void putFrame(Image image) {
        this.image = image;
        frame.repaint();
    }

    public void start() {
        frame = new JFrame("Output!");
        frame.setLocationByPlatform(true);
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setMinimumSize(new Dimension(180,100));
        
        JPanel imagePane = new JPanel() {
            @Override
            protected void paintComponent(Graphics g) {
                super.paintComponent(g);
                synchronized (OutputSimulator.this) {
                    if (image != null) {
                        g.drawImage(image, 0, 0, getWidth(), getHeight(), null);
                    }
                }
            }
        };
        
        frame.setContentPane(imagePane);
        frame.pack();
        frame.setVisible(true);
    }

    public void stop() {
        image = null;
        frame.dispose();
    }

}
