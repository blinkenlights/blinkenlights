/*
 * Created on Oct 3, 2008
 */
package de.blinkenlights.game.mole;

import java.awt.Color;
import java.awt.Graphics2D;

public class Hammer {

    private final int width;
    private final int height;
    private int ttl;
    
    public Hammer(int width, int height) {
        this.width = width;
        this.height = height;
        this.ttl = 2;
    }
    
    public boolean nextFrame(Graphics2D g, long when) {
        g.setXORMode(Color.BLACK);
        g.fillRect(0, 0, width, height);
        ttl--;
        return ttl >= 0;
    }

}
