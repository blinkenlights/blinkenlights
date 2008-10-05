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

import util.Util;
import de.blinkenlights.game.BlinkenGame;
import de.blinkenlights.game.FrameInfo;
import de.blinkenlights.game.GameConfigurationException;
import de.blinkenlights.game.GameContext;

public class Frogger implements BlinkenGame {
    
    public static final int MIN_STREAM_WIDTH = 4;
    
    public static final int MAX_STREAM_WIDTH = 8;
    
    public static final int MAX_STREAM_DELAY = 4;
    
    private int streamWidth;
    
    private int streamDelay;

    private Frog frog;
    
    private int width;
    
    private int height;
    
    private int numStreams;
    
    private int furthestStream;
    
    private int score;
    
    private List<Stream> streams;
    
    public void gameEnding(Graphics2D g) {
        Util.paintGameOver(g, score);
    }

    public void gameStarting(GameContext context) {
        context.setFramesPerSecond(3);
        streamDelay = MAX_STREAM_DELAY;
        streamWidth = MAX_STREAM_WIDTH;
        width = context.getPlayfieldWidth();
        height = context.getPlayfieldHeight();

        streams = new ArrayList<Stream>();
        streams.clear();
        frog = new Frog(this, new Point(streamWidth*(numStreams + 1), height/2));
        createStreams();
        score = 0;
    }

    private void createStreams() {
        if (streamWidth > MIN_STREAM_WIDTH) {
            streamWidth--;
        } else if (streamWidth == MIN_STREAM_WIDTH && streamDelay > 0) {
            streamDelay--;
        }
        numStreams = width/streamWidth - 2; //need ground
        boolean flowUp = true;
        frog.setFrogHead(new Point(streamWidth*(numStreams + 1), height/2));
        
        streams.clear();
        for (int i = 0; i < numStreams; i++) {
            streams.add(new Stream(this, flowUp, streamWidth*(i+1), height, streamDelay));
            flowUp = !flowUp;
        }
        furthestStream = frog.getFrogLeftEdge()/streamWidth - 1;
        System.out.println("Furthest stream " + furthestStream);
    }

    public boolean nextFrame(Graphics2D g, FrameInfo frameInfo) {
        Character input = frameInfo.getUserInput();
        if (input == null) {
            //no-op
        } else if (input == '4') {
            frog.move(Frog.Direction.LEFT);
        } else if (input == '6') {
            frog.move(Frog.Direction.RIGHT);
        } else if (input == '2') {
            frog.move(Frog.Direction.UP);
        } else if (input == '8') {
            frog.move(Frog.Direction.DOWN);
        }
        
        int streamId = frog.getFrogLeftEdge()/streamWidth - 1;
        boolean streamMoved = false;
        for (int i = 0; i < streams.size(); i++) {
            Stream s = streams.get(i);
            boolean tempStreamMoved = s.tick();
            if (i == streamId) {
                streamMoved = tempStreamMoved;
            }
        }
        
        System.out.println("stream id " + streamId + " furthest stream " + furthestStream);
        
        if (streamId < furthestStream) {
            score++;
            System.out.println("Score " + score);
            furthestStream = streamId;
        }
        if (streamId < streams.size() && streamId >= 0) {
            List<Boolean> currentStream = streams.get(streamId).getStreamLines();
            if (streamMoved) {
                if (streams.get(streamId).isFlowingUp()) {
                    frog.move(Frog.Direction.UP);
                } else {
                    frog.move(Frog.Direction.DOWN);
                }
            }
            if (frog.getTopEdge() < 0 || frog.getBottomEdge() >= height) {
                return false;
            }
            for (int i = frog.getTopEdge(); i <= frog.getBottomEdge(); i++) {
                if(!currentStream.get(i)) {
                    return false;
                }
            }
        } else if (streamId < 0) {
            createStreams();
        }
        
        
        g.clearRect(0, 0, width, height);
        g.fillRect(0, 0, streamWidth - 1, height);
        g.fillRect(streamWidth*(numStreams + 1), 0, streamWidth - 1, height);
        
        for (Stream s : streams) {
            s.paint(g);
        }
        
        frog.paint(g);
        return true;
    }
    
    public int getStreamWidth() {
        return streamWidth;
    }
    
    public static void main(String[] args) throws GameConfigurationException {
        Frogger game = new Frogger();
        GameContext context = new GameContext(game);
        context.start();
    }

}
