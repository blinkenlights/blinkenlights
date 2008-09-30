package de.blinkenlights.bmix.monitor;

import java.net.SocketException;

import de.blinkenlights.bmix.mixer.BLImage;
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
				null, 0,
				AlphaMode.OPAQUE, null, null, BLPacketReceiver.DEFAULT_TIMEOUT);
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