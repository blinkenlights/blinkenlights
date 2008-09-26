/*
 * Created on Sep 25, 2008
 */
package de.blinkenlights.game;

public interface UserInputSource {

    /**
     * Returns the most recently typed character from the user. If the user
     * has not typed a new character since last time this method was called,
     * returns null.
     */
    Character getKeystroke();
    
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
     * Blocks the calling thread until the game should start. If the game
     * should not start and the input client is being destroyed, throws
     * an exception.
     * 
     * @throws InterruptedException If the thread is interrupted while waiting
     * for the game to start. 
     */
    void waitForGameStart() throws InterruptedException;

}
