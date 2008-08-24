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
