/*
 * Created on Sep 29, 2008
 *
 * This code belongs to SQL Power Group Inc.
 */
package frogger;

import java.awt.Graphics2D;
import java.awt.Point;
import java.util.ArrayList;
import java.util.List;

public class Stream {
    
    private int xpos;
    
    private List<Boolean> streamLines;
    
    private int logCountdown;
    
    private boolean drawLog;
    
    private boolean flowUp;
    
    private final int delay;
    
    private int delayCountdown;

    private Frogger game;
    
    public Stream(Frogger parent, boolean flowUp, int xpos, int height, int maxDelay) {
        game = parent;
        this.flowUp = flowUp;
        this.xpos = xpos;
        streamLines = new ArrayList<Boolean>();
        logCountdown = 0;
        drawLog = false;
        
        delay = (int) (Math.random()*maxDelay) + 1;
        delayCountdown = delay;
        
        for (int i = 0; i < height; i++) {
            if (logCountdown == 0) {
                drawLog = !drawLog;
                if (drawLog) {
                    logCountdown = (int) (game.getStreamWidth() + 1 + Math.random()*game.getStreamWidth());
                } else {
                    logCountdown = (int) (1 + game.getStreamWidth() * Math.random());
                }
            }
            
            if (flowUp) {
                streamLines.add(drawLog);
            } else {
                streamLines.add(0, drawLog);
            }
            logCountdown--;
        }
    }
    
    /**
     * True if stream moved.
     */
    public boolean tick() {
        if (delayCountdown > 0) {
            delayCountdown--;
            return false;
        }
        delayCountdown = delay;
        if (logCountdown == 0) {
            drawLog = !drawLog;
            if (drawLog) {
                logCountdown = (int) (game.getStreamWidth() + 2 + Math.random()*game.getStreamWidth());
            } else {
                logCountdown = (int) (1 + game.getStreamWidth() * Math.random());
            }
        }
        
        if (flowUp) {
            streamLines.remove(0);
            streamLines.add(drawLog);
        } else {
            streamLines.remove(streamLines.size() - 1);
            streamLines.add(0, drawLog);
        }
        logCountdown--;
        return true;
    }
    
    public void paint(Graphics2D g) {
        for (int y = 0; y < streamLines.size(); y++) {
            if (streamLines.get(y)) {
                g.drawLine(xpos, y, xpos+game.getStreamWidth() - 2, y);
            }
        }
    }
    
    public List<Boolean> getStreamLines() {
        return streamLines;
    }
    
    public boolean isFlowingUp() {
        return flowUp;
    }
    

}
