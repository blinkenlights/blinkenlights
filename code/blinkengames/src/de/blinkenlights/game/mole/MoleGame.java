/*
 * Created on Oct 1, 2008
 */
package de.blinkenlights.game.mole;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.RenderingHints;

import de.blinkenlights.game.BlinkenGame;
import de.blinkenlights.game.FrameInfo;
import de.blinkenlights.game.GameContext;

public class MoleGame implements BlinkenGame {

    private static final long GAME_DURATION = 30000;

    private GameContext context;

    /**
     * The bounding rectangle of each square.
     */
    private Rectangle[] bounds;
    
    /**
     * The mole currently associated with each square.
     */
    private Mole[] moles;
    
    /**
     * The hammer currently bashing each square.
     */
    private Hammer[] hammers;
    
    /**
     * The number of moles that can be active at the same time. Gets larger as the game
     * progresses.
     */
    private int concurrentMoleLimit;

    private int squareWidth;

    private int squareHeight;

    private int score;
    
    public void gameStarting(GameContext context) {
        this.context = context;
        
        // make the biggest square that fits
        int width = context.getPlayfieldWidth();
        int height = context.getPlayfieldHeight();
        
        squareWidth = width / 3;
        squareHeight = height / 3;
        
        bounds = new Rectangle[9];
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                int idx = i + 3*j;
                bounds[idx] = new Rectangle(squareWidth*i, squareHeight*j, squareWidth, squareHeight);
            }
        }
        moles = new Mole[9];
        hammers = new Hammer[9];
        score = 0;
        concurrentMoleLimit = 1;
    }
    
    public void gameEnding(Graphics2D g) {
        g.setFont(Font.decode("System-10"));
        g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_OFF);
        String s = String.valueOf(score);
        FontMetrics fm = g.getFontMetrics();
        int w = (int) fm.getStringBounds(s, g).getWidth();
        g.setColor(Color.WHITE);
        System.out.println("Drawing score: " + s);
        g.drawString(s, context.getPlayfieldWidth()/2 - w/2, context.getPlayfieldHeight()/2 + fm.getAscent()/2);
        bounds = null;
        context = null;
    }

    public boolean nextFrame(Graphics2D g, FrameInfo frameInfo) {
        
        concurrentMoleLimit = (int) (frameInfo.getWhen() / 6000) + 1;
        
        Character key = frameInfo.getUserInput();
        if (key != null) {
            int idx;
            if (key == '1') idx = 0;
            else if (key == '2') idx = 1;
            else if (key == '3') idx = 2;
            else if (key == '4') idx = 3;
            else if (key == '5') idx = 4;
            else if (key == '6') idx = 5;
            else if (key == '7') idx = 6;
            else if (key == '8') idx = 7;
            else if (key == '9') idx = 8;
            else idx = -1;

            if (idx >= 0) {
                hammers[idx] = new Hammer(squareWidth, squareHeight);
                if (moles[idx] != null) {
                    moles[idx].whack();
                    score++; // TODO paint score
                    System.out.println("Score: " + score);
                }
            }
        }
        
        int currentMoleCount = 0;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                int idx = 3*i + j;
                Mole mole = moles[idx];
                Rectangle r = bounds[idx];
                if (mole != null) {
                    Graphics2D gg = (Graphics2D) g.create(r.x, r.y, r.width, r.height);
                    boolean thisMoleAlive = mole.nextFrame(gg, frameInfo.getWhen());
                    gg.dispose();
                    
                    if (thisMoleAlive) {
                        currentMoleCount++;
                    } else {
                        moles[idx] = null;
                        System.out.println("Removed finished mole at " + idx);
                    }
                }
                
                Hammer hammer = hammers[idx];
                if (hammer != null) {
                    Graphics2D gg = (Graphics2D) g.create(r.x, r.y, r.width, r.height);
                    boolean thisHammerAlive = hammer.nextFrame(gg, frameInfo.getWhen());
                    gg.dispose();
                    
                    if (!thisHammerAlive) {
                        hammers[idx] = null;
                    }
                }
            }
        }
        
        // add a mole if there aren't enough
        if (currentMoleCount < concurrentMoleLimit) {
            int idx = (int) (Math.random() * moles.length);
            Rectangle moleBounds = bounds[idx];
            moles[idx] = new Mole(new Dimension(moleBounds.width, moleBounds.height));
            System.out.println("Created new mole at " + idx);
        }
        
        return frameInfo.getWhen() < GAME_DURATION;
    }

    public static void main(String[] args) throws Exception {
        BlinkenGame game = new MoleGame();
        GameContext context = new GameContext(game);
        context.start();
    }
}
