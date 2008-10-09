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

package de.blinkenlights.bmix.main;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.Timer;
import java.util.TimerTask;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.XMLReader;

import de.blinkenlights.bmix.Version;
import de.blinkenlights.bmix.mixer.BLImage;
import de.blinkenlights.bmix.mixer.Output;
import de.blinkenlights.bmix.monitor.Monitor;
import de.blinkenlights.bmix.network.BLPacketReceiverThread;
import de.blinkenlights.bmix.statistics.FrameStatistics;
import de.blinkenlights.bmix.statistics.StatServer;

/**
 * This is the main class that drives the BMix program. The bmix config filename
 * should be specified on the command line. If it's not, "bmix.xml" will try to
 * be opened instead.
 */
public final class BMix extends Monitor {
	static final Logger logger = Logger.getLogger(BMix.class.getName());
	private BMixSession session;
	private StatServer statServer;
	
	private Timer configReloadTimer = new Timer();
	
    /**
	 * Creates a new BlinkenMix system.
	 * 
	 * @param configFilename the config file to use
	 * @throws SAXException 
	 * @throws ParserConfigurationException 
	 * @throws IOException 
	 */
	public BMix(String configFilename, boolean guiEnabled) throws ParserConfigurationException, SAXException, IOException {
	    super("bmix", 0, 0, 400, 300, guiEnabled);
	    statServer = new StatServer();
	    new Thread(statServer).start();
		File configFile = new File(configFilename);
		session = createSession(configFile);
		//configReloadTimer.schedule(new ConfigReloader(configFile), 1000, 1000);
	}

    /**
     * Creates a new bmix session by parsing the given config file.
     * 
     * @param configFile
     *            The configuration file, which must conform with the BMix DTD.
     * @return A new bmix session configured according to the given file
     * @throws ParserConfigurationException
     *             If there is a problem with the SAX parsing system
     * @throws SAXException
     *             If the parse fails
     * @throws IOException
     *             If the given file does not exist or can't be read
     */
	public static BMixSession createSession(File configFile)
			throws ParserConfigurationException, SAXException, IOException {
		BMixSAXHandler saxHandler = new BMixSAXHandler();
		SAXParserFactory spf = SAXParserFactory.newInstance();
		SAXParser sp = null;
		sp = spf.newSAXParser();
        EntityResolver resolver = new EntityResolver() {
            public InputSource resolveEntity(String publicId, String systemId) {
                if (publicId.equals("-//BMix//DTD BMix 1.0//EN")) {
                    InputStream in = BMix.class.getResourceAsStream(
                            "/de/blinkenlights/bmix/bmix.dtd");
                    return new InputSource(in);
                }
                return null;
            }
        };
        XMLReader reader = sp.getXMLReader();
        reader.setEntityResolver(resolver);
        reader.setContentHandler(saxHandler);
        reader.parse(new InputSource(configFile.getAbsolutePath()));	
		return saxHandler.getConfiguration();
	}
	
	/**
	 * This is the official, government-approved method to invoke bmix.
	 * </p><p>
	 * Valid Command-Line Arguments:
	 * <dl>
	 *  <dt><marquee>configFilename</marquee></dt><dd>the name of the config file to use</dd>
	 * </dl>
	 * @param args command line arguments
	 */
	public static void main(String[] args) throws Exception {
		int verbosity = 0;
		boolean gotFilename = false;
		boolean guiEnabled = true;
		
		String configFilename = "bmix.xml";

		for (String arg : args) {
			if (!arg.startsWith("-")) {
				if (!gotFilename) {
					configFilename = arg;
					gotFilename = true;
				} else {
					showHelp();
					return;
				}
			}
			else if ("-v".equals(arg)) {
				verbosity++;
			}
			else if ("-d".equals(arg)) {
				guiEnabled = false;
			} else {
				showHelp();
				return;
			}
			
			if (verbosity >= 50) {
				System.out.println("wow, we're getting really verbose now...");
			}
			
		}
		Level logLevel;
		if (verbosity == 0) {
			logLevel = Level.WARNING;
		}
		else if (verbosity == 1) {
			logLevel = Level.INFO;
		}
		else if (verbosity == 2) {
			logLevel = Level.FINE;
		}
		else if (verbosity >= 3) {
			logLevel = Level.FINEST;
		} else {
			System.err.println("Fatal Error: Invalid log verbosity: " + verbosity);
			return;
		}
		System.err.println("Setting log level to " + logLevel);
		Logger.getLogger("").setLevel(logLevel);
		for (Handler handler : Logger.getLogger("").getHandlers()) {
		    handler.setLevel(logLevel);
		}

		new BMix(configFilename, guiEnabled).start();		
	}
	
	private static void showHelp() {
	    System.out.println("BMix version " + Version.VERSION);
	    System.out.println(" Usage: bmix [-v] [-d] [config filename]");
	    System.out.println("  -v increase verbosity (use more for more verbosity)");
	    System.out.println("  -d disable gui");
	}

	// shuts down bmix cleanly and exits the jvm.
	@Override
	public void shutdown() {
		//logger.info("session shutting down.");
		//session.close();
		logger.info("shutdown complete.");
	}

    @Override
    protected BLImage getNextImage() {
        logger.entering("BMix", "getNextImage");
        
        session.waitForNewPacket();
        
        for (BLPacketReceiverThread t : session.getReceiverThreads()) {
            t.updateTargetLayers(session.getLayersForReceiver(t.getReceiver()));
        }
        
        session.getRootLayer().mixdown();
        
        for (Output output : session.getOutputs()) {
            try {
                output.send();
            } catch (Exception e) {
                logger.log(Level.WARNING, "Failed to send on output " + output,e);
            }
        }
        
        // send the statistics to the StatServer
        FrameStatistics frameStats = new FrameStatistics(session.getLayerInputs(), 
        		session.getRootLayer(), session.getLayerSources(), session.getOutputs());
        
		statServer.putFrameStatistic(frameStats);
		
        
        logger.exiting("BMix", "getNextImage", session.getRootLayer());
        return session.getRootLayer();
    }
    
    private class ConfigReloader extends TimerTask {

    	private final File fileToWatch;
		private long lastModified;

		public ConfigReloader(File fileToWatch) {
			this.fileToWatch = fileToWatch;
			lastModified = fileToWatch.lastModified();
    		
    	}
    	
		@Override
		public void run() {
			if (lastModified < fileToWatch.lastModified()) {
				// time to reload the session
				lastModified = fileToWatch.lastModified();
				session.close();
				try {
					session = BMix.createSession(fileToWatch);
				} catch (Throwable e) {
					// we should exit the JVM here so that bmix can be restarted immediately
					// if the session was not reloaded successfully.
					logger.log(Level.SEVERE,"something really bad happened reloading the configuraiton",e);
					System.exit(1);
				}
				
			}
		}
    	
    }
}
