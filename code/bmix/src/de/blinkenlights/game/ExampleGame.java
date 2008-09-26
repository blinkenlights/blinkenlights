/*
 * Created on Sep 25, 2008
 */
package de.blinkenlights.game;

import java.awt.Color;
import java.awt.Graphics2D;

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
        xOffset++;
        if (xOffset * 2 >= context.getPlayfieldWidth()) {
            xOffset = 0;
        }
        
        int height = context.getPlayfieldHeight();
        g.setColor(Color.WHITE);
        g.drawLine(centre - xOffset, 0, centre - xOffset, height);
        g.drawLine(centre + xOffset, 0, centre + xOffset, height);
        
        // just quit after 4 seconds
        return frameInfo.getWhen() < 4000;
    }

    public void gameEnding() {
        System.out.println("Game ending");
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
