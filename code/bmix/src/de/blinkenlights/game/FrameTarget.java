/*
 * Created on Sep 25, 2008
 */
package de.blinkenlights.game;

import java.awt.image.BufferedImage;
import java.io.IOException;

public interface FrameTarget {

    /**
     * Presents the given image to the user.
     * 
     * @throws IOException If the frame can't be sent to the target
     */
    void putFrame(BufferedImage image) throws IOException;
    
    /**
     * Performs any necessary startup routine for this frame target.
     */
    public void start();
    
    /**
     * Performs the shutdown routine which releases any resources held
     * by this frame target.
     */
    public void stop();

    /**
     * Notifies this frame target that the game is over. This frame target
     * will release any resources it had been using during the game.
     */
    public void gameEnding();
}
