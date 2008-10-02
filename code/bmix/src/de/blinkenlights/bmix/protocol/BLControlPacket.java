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
 * This is a BLControlPacket. This class is currently untested as no source
 * of these packets has been encountered.
 */
public class BLControlPacket implements BLPacket {
	private final byte pixelData[];

	/**
	 * Creates a new DeviceControl packet.
	 * 
	 * @param pixelData the pixel data to control
	 */
	public BLControlPacket(byte pixelData[]) {
		this.pixelData = pixelData;
		if (pixelData.length < 1) {
			throw new IllegalArgumentException("Array length must be > 0");
		}
	}


	/**
	 * Gets the pixel data for the device control packet.
	 * 
	 * @return the pixel data
	 */
	public byte[] getPixelData() {
		return pixelData;
	}
	
	
	/**
	 * Gets contents of this object in network protocol format.
	 * 
	 * @return the network protocol bytes
	 */
	public byte[] getNetworkBytes() {
		byte buf[] = new byte[8 + pixelData.length];
		buf[0] = 0x23;
		buf[1] = 0x54;
		buf[2] = 0x26;
		buf[3] = 0x66;
		buf[4] = (byte)((pixelData.length & 0xff00) >> 8);
		buf[5] = (byte)(pixelData.length & 0x00ff); 
		buf[6] = 0;
		buf[7] = 0; 
		System.arraycopy(pixelData, 0, buf, 8, pixelData.length);
		return buf;
	}
}
