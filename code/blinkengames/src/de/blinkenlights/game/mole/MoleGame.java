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

import javax.swing.ImageIcon;

import de.blinkenlights.game.BlinkenGame;
import de.blinkenlights.game.FrameInfo;
import de.blinkenlights.game.GameContext;

public class MoleGame implements BlinkenGame {

    private static final long GAME_DURATION = 30000;

    private final ImageIcon titleImage = new ImageIcon(MoleGame.class.getResource("title.png"));
    
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

    private Mode mode;
    
    private long INTRO_TIME = 3000;
    
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
        mode = Mode.INTRO;
    }
    
    public void gameEnding(Graphics2D g) {
        drawText(g, String.valueOf(score));
        bounds = null;
        context = null;
    }

    private void drawText(Graphics2D g, String s) {
        g.setFont(Font.decode("System-10"));
        g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_OFF);
        FontMetrics fm = g.getFontMetrics();
        int w = (int) fm.getStringBounds(s, g).getWidth();
        g.setColor(Color.WHITE);
        g.drawString(s, context.getPlayfieldWidth()/2 - w/2, context.getPlayfieldHeight()/2 + fm.getAscent()/2);
    }

    private static enum Mode { INTRO, GAME };
    
    public boolean nextFrame(Graphics2D g, FrameInfo frameInfo) {
        if (mode == Mode.INTRO) {
            boolean continueIntro = nextIntroFrame(g, frameInfo);
            if (!continueIntro) {
                mode = Mode.GAME;
            }
            return true;
        } else {
            return nextInGameFrame(g, frameInfo);
        }
    }
    
    private boolean nextIntroFrame(Graphics2D g, FrameInfo frameInfo) {
        if (frameInfo.getWhen() > INTRO_TIME - 600) {
            drawText(g, "1");
        } else if (frameInfo.getWhen() > INTRO_TIME - 1200) {
            drawText(g, "2");
        } else if (frameInfo.getWhen() > INTRO_TIME - 1800) {
            drawText(g, "3");
        } else {
            titleImage.paintIcon(null, g, context.getPlayfieldWidth()/2 - titleImage.getIconWidth()/2, 0);
        }
        return frameInfo.getWhen() < INTRO_TIME;
    }

    private boolean nextInGameFrame(Graphics2D g, FrameInfo frameInfo) {
        long when = frameInfo.getWhen() - INTRO_TIME;
        concurrentMoleLimit = (int) (when / 6000) + 1;
        
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
                    boolean thisMoleAlive = mole.nextFrame(gg, when);
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
                    boolean thisHammerAlive = hammer.nextFrame(gg, when);
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
        
        return when < GAME_DURATION;
    }

    public static void main(String[] args) throws Exception {
        BlinkenGame game = new MoleGame();
        GameContext context = new GameContext(game);
        context.start();
    }
}
