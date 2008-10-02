package de.blinkenlights.bmix.main;

import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.Logger;

import org.apache.commons.daemon.DaemonContext;

public class Daemon implements org.apache.commons.daemon.Daemon {
	private BMix bmix;

	public void destroy() {
		// no special cleanup required
	}

	public void init(DaemonContext arg0) throws Exception {
		int verbosity = 1;
		if (System.getProperty("debugLevel") != null) {
			verbosity = Integer.parseInt(System.getProperty("debugLevel"));
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
		
		bmix = new BMix("bmix.xml", false);	
	}

	public void start() throws Exception {
		bmix.start();
	}

	public void stop() throws Exception {
		bmix.shutdown();
	}

}
