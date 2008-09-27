/*
 * Created on Sep 25, 2008
 */
package de.blinkenlights.game;

import java.awt.Color;
import java.awt.Graphics2D;

/**
 * Simple example "game" that demonstrates how to use the Blinkengame API.
 * <p>
 * To launch this game, compile it:
 * <pre>
 *  % javac -cp blinkengame.jar ExampleGame.java
 * </pre>
 * then run it like this:
 * <pre>
 *  % java -cp blinkengame.jar:. ExampleGame
 * </pre>
 * (on windows, substitute a semilcolon for the colon).
 */
public class ExampleGame implements BlinkenGame {
    
    private GameContext context;
    private int centre;
    private int xOffset;
    
    public void gameStarting(GameContext gameContext) {
        System.out.println("Game starting");
        context = gameContext;
        centre = gameContext.getPlayfieldWidth() / 2;
        xOffset = 0;
    }

    public boolean nextFrame(Graphics2D g, FrameInfo frameInfo) {
        
        // TODO user input from frame info
        System.out.println("Frame@" + frameInfo.getWhen() + "ms. User Input="+frameInfo.getUserInput());
        Character input = frameInfo.getUserInput();
        if (input != null) {
            xOffset++;
        }
        if (xOffset * 2 >= context.getPlayfieldWidth()) {
            xOffset = 0;
        }
        
        int height = context.getPlayfieldHeight();
        g.setColor(Color.WHITE);
        g.drawLine(centre - xOffset, 0, centre - xOffset, height);
        g.drawLine(centre + xOffset, 0, centre + xOffset, height);
        
        // quit after 60 seconds so people can't hog it
        return frameInfo.getWhen() < 60000;
    }

    public void gameEnding(Graphics2D g) {
        System.out.println("Game ending");
        g.drawLine(0, 0, context.getPlayfieldWidth(), context.getPlayfieldHeight());
        context = null;
        xOffset = 0;
        centre = 0;
    }
    
    public static void main(String[] args) throws GameConfigurationException {
        BlinkenGame game = new ExampleGame();
        GameContext context = new GameContext(game);
        context.start();
    }

}
