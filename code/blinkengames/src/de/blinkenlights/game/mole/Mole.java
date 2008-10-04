/*
 * Created on Oct 2, 2008
 */
package de.blinkenlights.game.mole;

import java.awt.Dimension;
import java.awt.Graphics2D;
import java.awt.RenderingHints;

import javax.swing.Icon;
import javax.swing.ImageIcon;

public class Mole {

    private static final Icon moleImage = new ImageIcon(Mole.class.getResource("smallmole.png"));
    private static final Icon moleImageBlink = new ImageIcon(Mole.class.getResource("smallmole-blink.png"));
    
    
    private static enum Mode { UP, DOWN, TEASE };
    
    private Mode mode = Mode.UP;
    
    /**
     * how many pixels above the ground is the top of the image
     */
    private int top = 0;
    
    /**
     * Used as the argument to sin() for rotating the mole side to side
     * once it's fully up
     */
    private double phase = 0.0;
    
    private long teaseStartTime;
    
    private final int MAX_HEIGHT = 5;
    
    private int teaseDuration = 2000;
    
    private Icon image = moleImage;
    private final Dimension size;
    
    /**
     * Creates a new mole that will paint within the given dimensions.
     * @param size
     */
    public Mole(Dimension size) {
        this.size = size;
    }

    /**
     * Paints this mole into the given graphics. The painting is always done
     * with an origin of (0,0) so the caller has to translate it accordingly
     * before calling.
     * 
     * @param g
     *            Graphics to paint into
     * @param when
     *            The game time in milliseconds
     * @return True if this mole is still alive (nextFrame should be called
     *         again) or false if this was the last time this mole should be
     *         painted.
     */
    public boolean nextFrame(Graphics2D g, long when) {
        
        boolean alive = true;
        
        switch (mode) {
        
        case UP:
            top++;
            if (top >= MAX_HEIGHT) {
                top = MAX_HEIGHT;
                mode = Mode.TEASE;
                teaseStartTime = when;
            }
            break;
            
        case DOWN:
            top--;
            if (top <= 0) {
                alive = false;
            }
            break;
            
        case TEASE:
//            phase += .4;
            if ( (when % 10) == 3 || (when % 10) == 5) {
                image = moleImageBlink;
            } else {
                image = moleImage;
            }
            if (when > (teaseStartTime + teaseDuration)) {
//                phase = 0;
                image = moleImage;
                mode = Mode.DOWN;
            }
        }

        g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
        g.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_BICUBIC);
        
        g.rotate(Math.sin(phase), image.getIconWidth()/2, image.getIconHeight());
        image.paintIcon(null, g, size.width/2 - image.getIconWidth()/2, MAX_HEIGHT - top);
        return alive;
    }

    public void whack() {
        mode = Mode.DOWN;
    }
    
}
