/*
 * Created on Oct 7, 2008
 */
package de.blinkenlights.game.slideshow;

import java.awt.Graphics2D;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferInt;
import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.logging.Logger;

import de.blinkenlights.bmix.main.BMovieSender;
import de.blinkenlights.bmix.movie.BLMovie;
import de.blinkenlights.bmix.movie.Frame;
import de.blinkenlights.game.BlinkenGame;
import de.blinkenlights.game.FrameInfo;
import de.blinkenlights.game.GameConfigurationException;
import de.blinkenlights.game.GameContext;

/*
 * TODO list:
 * -prevent game ending upon hangup
 * 
 */

/**
 * The Slideshow "game" isn't a game at all: it actually gives control
 * over the playback of one or more still images for the purpose of
 * making a Blinkenlights installation easier to photograph.
 */
public class Slideshow implements BlinkenGame {

    private static final Logger logger = Logger.getLogger(Slideshow.class.getName());
    
    /**
     * The list of movies. This is loaded from the context properties movie.[0-9]*.
     */
    private List<BLMovie> movies;
    
    /**
     * The index of the current movie in the movies list.
     */
    private int currentMovie;
    
    /**
     * The index of the current frame in the current movie.
     */
    private int currentFrame;
    
    /**
     * The sender that sends the current frame to the 
     */
    private BMovieSender sender;
    
    private int brightnessAdjust = 0;
    
    public void gameStarting(GameContext context) {
        movies = new ArrayList<BLMovie>();
        currentMovie = 0;
        try {
            int i = 0;
            while (context.getProperty("movie." + i) != null) {
                File moviefile = new File(context.getProperty("movie." + i));
                if (moviefile.isDirectory()) {
                    for (File ent : moviefile.listFiles()) {
                        if (ent.getName().contains(".bml")) {
                            BLMovie movie = new BLMovie(ent.getPath());
                            if (movie.getNumFrames() > 0) { // XXX need proper file type detector in BLMovie
                                movies.add(movie);
                            }
                        }
                    }
                } else {
                    movies.add(new BLMovie(moviefile.getPath()));
                }
                i++;
            }
            logger.fine("Loaded " + movies.size() + " movies");
        } catch (Exception ex) {
            throw new RuntimeException(ex);
        }
    }

    /**
     * Controls:
     * <table border="1">
     *   <tr>
     *     <td>1<br>prev frame
     *     <td>2<br>first frame
     *     <td>3<br>next frame
     *   <tr>
     *     <td>4<br>prev movie
     *     <td>5<br>first movie
     *     <td>6<br>next movie
     *   <tr>
     *     <td>7<br>darker
     *     <td>8<br>reset brightness
     *     <td>9<br>brighter
     *   <tr>
     *     <td>*
     *     <td>0<br>stop slideshow and hang up
     *     <td>#
     * </table>
     */
    public boolean nextFrame(Graphics2D g, FrameInfo frameInfo) {
        Character key = frameInfo.getUserInput();
        if (key != null) {
            if (key == '1') {
                // prev frame
                currentFrame--;
                if (currentFrame < 0) {
                    currentFrame = movies.get(currentMovie).getNumFrames() - 1;
                }
            } else if (key == '2') {
                // first frame
                currentFrame = 0;
            } else if (key == '3') {
                // next frame
                currentFrame++;
                if (currentFrame >= movies.get(currentMovie).getNumFrames()) {
                    currentFrame = 0;
                }
            } else if (key == '4') {
                // prev movie
                currentFrame = 0;
                currentMovie--;
                if (currentMovie < 0) {
                    currentMovie = movies.size() - 1;
                }
            } else if (key == '5') {
                // first movie
                currentFrame = 0;
                currentMovie = 0;
            } else if (key == '6') {
                // next movie
                currentFrame = 0;
                currentMovie++;
                if (currentMovie >= movies.size()) {
                    currentMovie = 0;
                }
            } else if (key == '7') {
                brightnessAdjust -= 16;
            } else if (key == '8') {
                brightnessAdjust = 0;
            } else if (key == '9') {
                brightnessAdjust += 16;
            } else if (key == '0') {
                // terminate
                // TODO when call ended != terminate, have to do more here
                return false;
            }
        }
        
        Frame f = movies.get(currentMovie).getFrame(currentFrame);
        BufferedImage bi = new BufferedImage(f.getImageWidth(), f.getIconHeight(), BufferedImage.TYPE_INT_ARGB);
        f.fillBufferedImage(bi);
        int[] biPixels = ((DataBufferInt) bi.getRaster().getDataBuffer()).getData();
        int addme = (brightnessAdjust & 0xff) << 16 | (brightnessAdjust & 0xff) << 8 | (brightnessAdjust & 0xff); // FIXME
        for (int i = 0; i < biPixels.length; i++) {
            int red   = (biPixels[i] >> 16) & 0xff;
            int green = (biPixels[i] >> 8) & 0xff;
            int blue  = (biPixels[i]) & 0xff;
            red = Math.min(Math.max(0, red + brightnessAdjust), 0xff);
            green = Math.min(Math.max(0, green + brightnessAdjust), 0xff);
            blue = Math.min(Math.max(0, blue + brightnessAdjust), 0xff);
            biPixels[i] = (0xff << 24) | (red << 16) | (green << 8) | blue;
        }
        g.drawImage(bi, 0, 0, null);
        return true;
    }

    public void gameEnding(Graphics2D g) {
        // nothing to do
    }

    public static void main(String[] args) throws GameConfigurationException {
        Slideshow game = new Slideshow();
        GameContext context = new GameContext(game);
        context.start();
    }
    
}
