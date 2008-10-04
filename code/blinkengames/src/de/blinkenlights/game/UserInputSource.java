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

public interface UserInputSource {

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
     * Returns the most recently typed character from the user. If the user
     * has not typed a new character since last time this method was called,
     * returns null.
     */
    Character getKeystroke();
    
    /**
     * Blocks the calling thread until the game should start. If the game
     * should not start and the input client is being destroyed, throws
     * an exception.
     * 
     * @throws InterruptedException If the thread is interrupted while waiting
     * for the game to start. 
     */
    void waitForGameStart() throws InterruptedException;

    /**
     * Returns true as long as the user is present. For the telephony implementation,
     * this will return false once the call is over.
     */
    boolean isUserPresent();
    
    /**
     * Provides notification that the game is ending. Callingt this method releases
     * any persistent resources (for example, a phone connection) that were held during
     * the game.
     */
    void gameEnding();
    
    void playBackgroundMusic(String musicName);
    
}
