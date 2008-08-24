package de.blinkenlights.bmix.monitor;

/**
 * This class represents the configuration of a NetworkStreamMonitor to
 * set the listen port, textual name, and size/position of the GUI monitor
 * on the screen.
 */
public class NetworkStreamMonitorConfig {
	int port;
	String name;
	int x;
	int y;
	int w;
	int h;
	

	/**
	 * Creates a NetworkStreamMonitorConfig object.
	 * 
	 * @param port the port number to listen on
	 * @param name the textual name of the monitor to use in the display
	 * @param x the x position on the screen
	 * @param y the y position on the screen
	 * @param w the width of the monitor frame
	 * @param h the height of the monitor frame
	 */
	public NetworkStreamMonitorConfig(int port, String name, int x, int y,
			int w, int h) {
		this.port = port;
		this.name = name;
		this.x = x;
		this.y = y;
		this.w = w;
		this.h = h;
	}


	public int getPort() {
		return port;
	}


	public String getName() {
		return name;
	}


	public int getX() {
		return x;
	}


	public int getY() {
		return y;
	}


	public int getW() {
		return w;
	}


	public int getH() {
		return h;
	}
}
