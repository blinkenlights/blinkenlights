package de.blinkenlights.bmix.protocol;

/**
 * This class represents a BLHeartbeat Packet. This class is currently 
 * untested as no source of these packets has been encountered.
 */
public class BLHeartbeatPacket implements BLPacket {
	private final int version;
	
	public static final int VERSION_NUMBER = 0;
	
	/**
	 * Creates a new BLHeartbeatPacket.
	 * 
	 * @param version the heartbeat version
	 */
	public BLHeartbeatPacket(int version) {
		this.version = version;
	}
	
	
	/**
	 * Gets the version.
	 * 
	 * @return the heartbeat version
	 */
	public int getVersion() {
		return version;
	}


	/**
	 * Gets contents of this object in network protocol format.
	 * 
	 * @return the network protocol bytes
	 */
	public byte[] getNetworkBytes() {
		byte buf[] = new byte[6];
		buf[0] = 0x42;
		buf[1] = 0x42;
		buf[2] = 0x42;
		buf[3] = 0x42;
		buf[4] = (byte)((version & 0xff00) >> 8);
		buf[5] = (byte)(version & 0x00ff);
		return buf;
	}	
}
