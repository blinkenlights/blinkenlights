package de.blinkenlights.bmix.util;

import java.awt.image.BufferedImage;

/**
 * This class provides some useful utilities.
 */
public class Utils {

	/**
	 * Converts a byte[] to a hex String.
	 * 
	 * @param buf the byte[] to convert
	 * @param len the number of bytes to convert
	 * @return a String representing the buffer, or an error message
	 */
	public static String bufHexToString(byte buf[], int len) {
		if(len < 1) {
			return "len is < 0";
		}
		if(buf == null || buf.length < 1) {
			return "buf is null or length < 1";
		}
		String str = "";
		for(int i = 0; i < len; i ++) {
			str += Integer.toHexString(buf[i] & 0xff) + "  ";
		}
		return str;
	}
	
	
	/**
	 * Converts bytes to address string with '.' separators.
	 * 
	 * @param addr the addr byte arraw
	 * @return a formatted String
	 */
	public static String byteArrayToAddressString(byte addr[]) {
		return (addr[0] & 0xff) + "." + (addr[1] & 0xff) + "." + 
			(addr[2] & 0xff) + "." + (addr[3] & 0xff);
	}


	/**
	 * Converts bytes to address string with '.' separators.
	 * 
	 * @param addrA the addrA byte
	 * @param addrB the addrB byte
	 * @param addrC the addrC byte
	 * @param addrD the addrD byte
	 * @return a formatted String
	 */
	public static String byteToAddressString(int addrA, int addrB, int addrC, int addrD) {
		return addrA + "." + addrB + "." + addrC + "." + addrD;
	}
	
	
	/**
	 * Sets all pixels of the given image to black and fully transparent
	 * (RGBA 0).
	 * 
	 * @param bi The image to clear.
	 */
	public static void clearImage(BufferedImage bi) {
        for (int x = 0; x < bi.getWidth(); x++) {
            for (int y = 0; y < bi.getHeight(); y++) {
                bi.setRGB(x, y, 0);
            }
        }
	}
}
