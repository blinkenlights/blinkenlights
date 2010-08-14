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

package de.blinkenlights.bmix.monitor;

import java.net.SocketException;

import de.blinkenlights.bmix.mixer.BLImage;
import de.blinkenlights.bmix.mixer.ScaleMode;
import de.blinkenlights.bmix.network.BLPacketReceiver;
import de.blinkenlights.bmix.network.BLPacketReceiver.AlphaMode;
import de.blinkenlights.bmix.protocol.BLFramePacket;
import de.blinkenlights.bmix.protocol.BLPacket;

/**
 * This class opens a BLPacketReceiver to listen on a port
 * and then displays the BL frame data on the screen.
 */
public class NetworkStreamMonitor extends Monitor {
	private BLPacketReceiver bpr;

	/**
	 * Creates a new NetworkStreamMonitor.
	 * 
	 * @param nc the NetworkStreamMonitorConfig to use for configuration
	 */
	public NetworkStreamMonitor(NetworkStreamMonitorConfig nc) throws SocketException {
		super(nc.getName(), nc.getX(), nc.getY(), nc.getW(), nc.getH(), true);		
		this.bpr = new BLPacketReceiver(
				"receiver", nc.getPort(), null,
				null, 0, ScaleMode.NEAREST_NEIGHBOUR,
				AlphaMode.OPAQUE, null, null, BLPacketReceiver.DEFAULT_TIMEOUT, null);
	}

	
	@Override
	protected BLImage getNextImage() {
		for (;;) {
			BLPacket bp = bpr.receive();
			if (bp instanceof BLFramePacket) {
				return (BLImage) bp;
			}
		}
	}
	
	public void close() {
	    bpr.close();
	}
}