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
 * This class represents a BLSetupPacket. This class is currently untested
 * as no source of these packets has been encountered.
 */
public class BLSetupPacket implements BLPacket {
	private final int id;
	private final int width;
	private final int height;
	private final int channels;
	private final int pixels;
	private final byte pixelData[];
	
	
	/**
	 * Creates a new BLSetupPacket.
	 * 
	 * @param id the device ID
	 * @param width display width
	 * @param height display height
	 * @param channels the number of colour channels
	 * @param pixels the number of total ports used
	 * @param pixelData the initialization data
	 */
	public BLSetupPacket(int id, int width, int height, int channels,
			int pixels, byte pixelData[]) {
		this.id = id & 0xff;
		this.width = width;
		this.height = height;
		this.channels = channels;
		this.pixels = pixels;
		this.pixelData = pixelData;
	}


	public int getId() {
		return id;
	}


	public int getWidth() {
		return width;
	}


	public int getHeight() {
		return height;
	}


	public int getChannels() {
		return channels;
	}


	public int getPixels() {
		return pixels;
	}


	public byte[] getPixelData() {
		return pixelData;
	}


	/**
	 * Gets contents of this object in network protocol format.
	 * 
	 * @return the network protocol bytes
	 */
	public byte[] getNetworkBytes() {
		byte buf[] = new byte[12 + (width * height * channels)];
		buf[0] = 0x23;
		buf[1] = 0x42;
		buf[2] = (byte)0xfe;
		buf[3] = (byte)0xed;
		buf[4] = (byte)id;
		buf[5] = 0; 
		buf[6] = 0;
		buf[7] = 0; 
		buf[8] = (byte)((height & 0xff00) >> 8);
		buf[9] = (byte)(height & 0x00ff); 
		buf[10] = (byte)((width & 0xff00) >> 8);
		buf[11] = (byte)(width & 0x00ff);
		buf[12] = (byte)((channels & 0xff00) >> 8);
		buf[13] = (byte)(channels & 0x00ff);
		buf[14] = (byte)((pixels & 0xff00) >> 8);
		buf[15] = (byte)(pixels & 0x00ff);
		System.arraycopy(pixelData, 0, buf, 16, pixelData.length);
		return buf;	}
}
