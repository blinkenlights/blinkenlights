/* 
 * This file is part of BMix.
 *
 *    BMix is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 * 
 *    BMix is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with BMix.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */
package de.blinkenlights.game;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.image.BufferedImage;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.InetAddress;
import java.util.Properties;
import java.util.logging.Level;
import java.util.logging.Logger;

public class GameContext implements Runnable {
    
    /**
     * Maximum run time for a game, in milliseconds. This value is enforced
     * by the game context.
     * <p>
     * Controlled by the <code>game.maxTime</code> configuration property.
     * Default value is no limit.
     */
    private long maxGameTime = Long.MAX_VALUE;

    Logger logger = Logger.getLogger(GameContext.class.getName());
    
    private final BlinkenGame game;
    
    private int playfieldWidth;

    private int playfieldHeight;
    
    private double framesPerSecond;

    private FrameTarget bmixClient;

    private UserInputSource inputClient;

	private Properties config;
    
    
    public GameContext(BlinkenGame game) throws GameConfigurationException {
        this.game = game;
        
        try {
        	Properties config = new Properties();
        	FileInputStream propertiesIn = new FileInputStream("blinkengame.properties");
        	config.load(propertiesIn);
        	propertiesIn.close();

        	processConfiguration(config);
        } catch (IOException ex) {
        	throw new GameConfigurationException(ex);
        }
    }

	/**
	 * Processes the given configuration, loading the game class dynamically.
	 * The game class must be specified in the given properties file under key
	 * "game.class".
	 * 
	 * @param gameConfig
	 *            The game's properties.
	 * @throws GameConfigurationException
	 *             If any mandatory properties are missing or invalid in
	 *             the given properties bundle.
	 */
    public GameContext(Properties gameConfig) throws GameConfigurationException {
    	processConfiguration(gameConfig);
        try {
        	String gameClassName = gameConfig.getProperty("game.class");
        	if (gameClassName == null) {
        		throw new GameConfigurationException("Missing mandatory property game.class");
        	}
        	Class<?> gameClass = Class.forName(gameClassName);
        	game = gameClass.asSubclass(BlinkenGame.class).newInstance();
        } catch (Exception ex) {
        	throw new GameConfigurationException(ex);
        }
	}

	/**
     * Reads the configuration from a properties file called
     * blinkengame.properties in the current directory.
     * 
     * @throws GameConfigurationException
     *             if the file "blinkengame.properties" is not in the current
     *             directory, it cannot be read, or it does not contain the
     *             required set of property values.
     */
    private void processConfiguration(Properties config) throws GameConfigurationException {
        try {
        	this.config = config;
        	
        	maxGameTime = Long.parseLong(config.getProperty("game.maxTime"));
        	
            playfieldWidth = Integer.parseInt(config.getProperty("playfield.width"));
            playfieldHeight = Integer.parseInt(config.getProperty("playfield.height"));

            framesPerSecond = Double.parseDouble(config.getProperty("playfield.fps"));
            
            if (Boolean.valueOf(config.getProperty("bmix.simulate"))) {
                bmixClient = new OutputSimulator();
            } else {
                InetAddress bmixHost = InetAddress.getByName(config.getProperty("bmix.host"));
                int bmixPort = Integer.parseInt(config.getProperty("bmix.port"));
                bmixClient = new BMixClient(bmixHost, bmixPort);
            }
            
            if (Boolean.valueOf(config.getProperty("telephony.simulate"))) {
                inputClient = new TelephoneSimulator();
            } else {
                InetAddress bltHost = InetAddress.getByName(config.getProperty("telephony.host"));
                int bltPort = Integer.parseInt(config.getProperty("telephony.port"));
                String did = config.getProperty("telephony.did");
                inputClient = new BLTClient(bltHost, bltPort, did);
            }
            
        } catch (Exception ex) {
            throw new GameConfigurationException(ex);
        }
    }
    
    /** 
     * This returns a property from the blinkengame.properties file.
     * 
     * @param property the name of the property to return
     * @return the property
     */
    public String getProperty(String property) {
		return config.getProperty(property);
	}

    /**
     * Starts the game context, which causes it to register with the telephony
     * server and begin to accept calls.
     */
    public void start() {
        bmixClient.start();
        inputClient.start();
        try {
            for (;;) {
                try {
                    inputClient.waitForGameStart();
                    doGame();
                } catch (Exception ex) {
                    ex.printStackTrace();
                }
                
                try {
                	Thread.sleep(500);
                } catch (InterruptedException ex) {
                	// ok, we'll just retry sooner
                }
            }
        } finally {
            bmixClient.stop();
            inputClient.stop();
        }
    }
    
    public void run() {
    	start();
    }
    
    private void doGame() {
        BufferedImage bi = new BufferedImage(playfieldWidth, playfieldHeight, BufferedImage.TYPE_INT_ARGB);
        try {
            game.gameStarting(this);
            long startTime = System.currentTimeMillis();
            for (;;) {
                long when = System.currentTimeMillis() - startTime;
                if (when > maxGameTime) {
                    logger.finer("Stopping game because time limit is exhausted");
                    break;
                }
                FrameInfo frameInfo = new FrameInfo(inputClient.getKeystroke(), when);
                Graphics2D g = (Graphics2D) bi.getGraphics();
                try {
                    g.setColor(Color.BLACK); // TODO configurable background colour
                    g.fillRect(0, 0, playfieldWidth, playfieldHeight);
                    g.setColor(Color.WHITE);
                    boolean keepGoing = game.nextFrame(g, frameInfo);
                    bmixClient.putFrame(bi);
                    if (!keepGoing) {
                        logger.info("Stopping at game's request (nextFrame returned false)");
                        break;
                    }
                    if (!inputClient.isUserPresent()) {
                        logger.info("Stopping because user input source went dead");
                        break;
                    }
                    Thread.sleep((long) ((1.0 / framesPerSecond) * 1000));
                } catch (InterruptedException e) {
                    // not a biggie
                } catch (IOException e) {
                    logger.log(Level.WARNING, "Terminating game because of IO problem", e);
                } finally {
                    g.dispose();
                }
            }
            
            // let the game draw a final frame
            Graphics2D g = (Graphics2D) bi.getGraphics();
            g.setColor(Color.BLACK); // TODO configurable background colour
            g.fillRect(0, 0, playfieldWidth, playfieldHeight);
            g.setColor(Color.WHITE);
            try {
                game.gameEnding(g);
                bmixClient.putFrame(bi);
            } catch (IOException ex) {
                logger.log(Level.WARNING, "Last frame could not be sent", ex);
            } finally {
                g.dispose();
            }

            // TODO last frame delay? (config file)
            // currently handled by a long timeout in the bmix input
            
        } finally {
            inputClient.gameEnding();
            bmixClient.gameEnding();
        }
    }

    public int getPlayfieldWidth() {
        return playfieldWidth;
    }

    public int getPlayfieldHeight() {
        return playfieldHeight;
    }

    public double getFramesPerSecond() {
        return framesPerSecond;
    }

    public void setFramesPerSecond(double framesPerSecond) {
        this.framesPerSecond = framesPerSecond;
    }
    
    public void startBackgroundMusic(String musicName) {
    	inputClient.playBackgroundMusic(musicName);
    }
}
