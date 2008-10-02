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
