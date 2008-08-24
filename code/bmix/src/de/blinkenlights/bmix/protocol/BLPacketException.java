package de.blinkenlights.bmix.protocol;

/**
 * This class represents a BLPacketException which is thrown if there
 * is a parse error while parsing network bytes.
 */
public class BLPacketException extends Exception {
	private static final long serialVersionUID = 1L;
	
	/**
	 * Creates a new BLPacketException.
	 * 
	 * @param msg the error message
	 */
	public BLPacketException(String msg) {
		super(msg);
	}
}
