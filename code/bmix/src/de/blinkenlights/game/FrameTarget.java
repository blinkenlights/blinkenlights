/*
 * Created on Sep 25, 2008
 */
package de.blinkenlights.game;

import java.awt.Image;

public interface FrameTarget {

    /**
     * Presents the given image to the user.
     */
    void putFrame(Image image);
    
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
