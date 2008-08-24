package de.blinkenlights.bmix.network;

/**
 * This class represents a network exception.
 */
public class BLNetworkException extends Exception {
	private static final long serialVersionUID = 1L;

	/**
	 * Creates a new BLNetworkException.
	 * 
	 * @param msg the error message
	 */
	public BLNetworkException(String msg) {
		super(msg);
	}
}
