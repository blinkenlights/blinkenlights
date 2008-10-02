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
