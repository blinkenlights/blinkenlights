/* 
 * This file is part of BMix.
 *
 *    BMix is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 * 
 *    BMix is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with BMix.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */
package de.blinkenlights.game;

import java.awt.Graphics2D;

/**
 * The interface that a game implements. The methods on this interface
 * are used by the GameContext to start, play, and stop a game.
 */
public interface BlinkenGame {

    /**
     * Called when a new game is starting. Instances of BlinkenGame may
     * be reused, but only one game will be in play on each instance
     * at a time. In other words, {@link #gameEnding(Graphics2D)} will
     * be called before this gameStarting() method is called again.
     * 
     * @param context The game context that is starting the game. Provides
     * information such as playfield dimensions, frame rate, and so on.
     */
    void gameStarting(GameContext context);

    /**
     * Produces the next visible frame of the game by drawing into the given
     * Graphics2D object.
     * 
     * @param g
     *            The graphics to draw the game's next frame with. Initialized
     *            to draw in white; all existing pixels of the image are black.
     * @param frameInfo
     *            Information about this frame, such as the number of
     *            milliseconds since the beginning of the game.
     * @return True if there should be another frame in this game; false if the
     *         game is over.
     */
    boolean nextFrame(Graphics2D g, FrameInfo frameInfo);

    /**
     * Called by the GameContext to indicate the game has ended. A game can end
     * for several reasons: nextFrame() may have returned false; the user may
     * have disconnected the input device (for example, ended the call on their
     * mobile phone) or the system may be stopping the game to make way for some
     * higher-priority content.
     * 
     * @param g
     *            The graphics to draw the game's final screen in. This could be
     *            used for a "you win" or "you lose" screen and to display the
     *            user's score. There can only be one final screen, and it will
     *            appear for some amount of time beyond the control of this game
     *            (it will normally be set to around 5 seconds).
     */
    void gameEnding(Graphics2D g);
    
}
