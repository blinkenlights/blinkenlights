package processing.blinkenlights;

/**
 * This class is the superclass for all BLPacket types.
 */
public interface BLPacket {

	/**
	 * Gets contents of this object in network protocol format.
	 * 
	 * @return the network protocol bytes
	 */
	public byte[] getNetworkBytes();
}
