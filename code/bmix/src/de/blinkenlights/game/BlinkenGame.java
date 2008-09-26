/*
 * Created on Sep 25, 2008
 */
package de.blinkenlights.game;

import java.awt.Graphics2D;

public interface BlinkenGame {

    void gameStarting(GameContext context);
    boolean nextFrame(Graphics2D g, FrameInfo frameInfo);
    void gameEnding();
    
}
