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
 * This class represents a BLHeartbeat Packet. This class is currently 
 * untested as no source of these packets has been encountered.
 */
public class BLHeartbeatPacket implements BLPacket {
	private final int version;
	
	public static final int VERSION_NUMBER = 0;

    private final String fromHost;

    private final int fromPort;
	
	/**
	 * Creates a new BLHeartbeatPacket for sending out. Received host and port
	 * information is defaulted to null and 0 respectively.
	 * 
	 * @param version the heartbeat version
	 */
	public BLHeartbeatPacket(int version) {
		this.version = version;
		fromHost = null;
		fromPort = 0;
	}
	
	
	public BLHeartbeatPacket(int version, String fromHost, int fromPort) {
        this.version = version;
        this.fromHost = fromHost;
        this.fromPort = fromPort;
    }


    /**
	 * Gets the version.
	 * 
	 * @return the heartbeat version
	 */
	public int getVersion() {
		return version;
	}

	public String getFromHost() {
        return fromHost;
    }
	
	public int getFromPort() {
        return fromPort;
    }

	/**
	 * Gets contents of this object in network protocol format.
	 * 
	 * @return the network protocol bytes
	 */
	public byte[] getNetworkBytes() {
		byte buf[] = new byte[12];
		buf[0] = 0x42;
		buf[1] = 0x42;
		buf[2] = 0x42;
		buf[3] = 0x42;
		buf[4] = (byte)((version & 0xff00) >> 8);
		buf[5] = (byte)(version & 0x00ff);
		buf[6] = 0x00;
		buf[7] = 0x00;
		buf[8] = 0x00;
		buf[9] = 0x00;
		buf[10] = 0x00;
		buf[11] = 0x00;
		return buf;
	}	
}
