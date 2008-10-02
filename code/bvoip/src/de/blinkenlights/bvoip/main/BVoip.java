package de.blinkenlights.bvoip.main;

import java.io.FileInputStream;
import java.io.IOException;
import java.net.InetAddress;
import java.util.Properties;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.Logger;

import de.blinkenlights.bvoip.ChannelList;
import de.blinkenlights.bvoip.asterisk.AGIServer;
import de.blinkenlights.bvoip.blt.BLTClientManager;

public class BVoip {
	private static final Logger logger = Logger.getLogger(BVoip.class.getName());
	private static int listenPort;
	
	
	/**
	 * Creates and starts the server to listen for voip events.
	 */
	public BVoip() {
		logger.entering(getClass().getName(), getClass().getSimpleName());
		
		ChannelList channelList = new ChannelList(2);
	
		new Thread(new BLTClientManager(listenPort,channelList)).start();
		
		// TODO - make configurable port number
		AGIServer agiServer = new AGIServer(4545, channelList);
		agiServer.run();
		
		
	}

	
	public static void main(String args[]) throws IOException {
		
		Properties properties = new Properties();
		FileInputStream in = new FileInputStream("bvoip.properties");
		properties.load(in);
		in.close();
				
		if (properties.get("listenPort") == null) {
			throw new IllegalArgumentException("missing required property 'listenPort'");
		}
		listenPort = Integer.parseInt(properties.getProperty("listenPort"));
		
		
		int verbosity = 2;
		
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
		
		new BVoip();
	}
}
