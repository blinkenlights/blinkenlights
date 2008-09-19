package de.blinkenlights.bmix.main;

import org.apache.commons.daemon.DaemonContext;

public class Daemon implements org.apache.commons.daemon.Daemon {

	private String[] arguments;

	@Override
	public void destroy() {
		// no special cleanup required
	}

	@Override
	public void init(DaemonContext arg0) throws Exception {
		arguments = arg0.getArguments();
		
	}

	@Override
	public void start() throws Exception {
		// BMix.main starts a thread and returns...
		BMix.main(arguments);
	}

	@Override
	public void stop() throws Exception {
		// no special cleanup....		
		// TODO: possibly blank the screen?
	}

}
