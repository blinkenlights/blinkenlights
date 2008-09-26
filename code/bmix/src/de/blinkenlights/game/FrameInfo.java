/*
 * Created on Sep 25, 2008
 */
package de.blinkenlights.game;

public class FrameInfo {

    /**
     * Number of milliseconds since the beginning of the game.
     */
    private final long when;
    
    private final Character userInput;
    
    /**
     * @param userInput
     * @param when
     */
    public FrameInfo(Character userInput, long when) {
        super();
        this.userInput = userInput;
        this.when = when;
    }
    
    public long getWhen() {
        return when;
    }
    
    public Character getUserInput() {
        return userInput;
    }
    
}
