/*
 * Created on Sep 25, 2008
 */
package de.blinkenlights.game;

/**
 * Thrown when the game context cannot start due to a missing, inadequate,
 * or faulty configuration file.
 */
public class GameConfigurationException extends Exception {
    
    public GameConfigurationException(String message) {
        super(message);
    }
    
    public GameConfigurationException(String message, Throwable cause) {
        super(message, cause);
    }

    public GameConfigurationException(Throwable cause) {
        super(cause);
    }
}
