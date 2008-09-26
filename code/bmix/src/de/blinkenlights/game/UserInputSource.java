/*
 * Created on Sep 25, 2008
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
}
