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

package de.blinkenlights.bmix.network;

import java.awt.Color;
import java.awt.Point;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;

import de.blinkenlights.bmix.mixer.ScaleMode;
import de.blinkenlights.bmix.protocol.BLHeartbeatPacket;
import de.blinkenlights.bmix.protocol.BLPacket;
import de.blinkenlights.bmix.protocol.BLPacketFactory;

/**
 * This class is a BL packet receiver. It listens on a port and waits for
 * incoming packets. When a packet is received it returns a BLPacket of the
 * correct type or null if there was an error.
 */
public class BLPacketReceiver {

	public static enum AlphaMode {
		/** alpha channel ignored even if present */
		OPAQUE("opaque"),

		/** one colour is fully transparent; all others are opaque */
		CHROMA_KEY("chroma-key"),

		/** all pixels are full white, and opacity is set to brightness */
		BRIGHTNESS("brightness"),

		/** magically delicious undocumented mode */
		NATIVE("native");

		private String code;

		AlphaMode(String code) {
			this.code = code;
		}

		public static AlphaMode forCode(String code) {
			for (AlphaMode am : values()) {
				if (am.code.equals(code)) {
					return am;
				}
			}
			throw new IllegalArgumentException("Unknown code: " + code);
		}

		@Override
		public String toString() {
			return code;
		}
	}

	private final static Logger logger = Logger
			.getLogger(BLPacketReceiver.class.getName());
	byte buf[] = new byte[65536];
	private final byte[] heartBeatBytes = new BLHeartbeatPacket(
			BLHeartbeatPacket.VERSION_NUMBER).getNetworkBytes();
	private long lastPacketReceiveTime = 0;
	private String lastSourceAddr = "null";

	/**
	 * The default address to listen on (binds to all local addresses).
	 */
	private static final InetAddress WILDCARD_ADDRESS;

	static {
		try {
			WILDCARD_ADDRESS = InetAddress.getByName("0.0.0.0");
		} catch (UnknownHostException e) {
			throw new RuntimeException(
					"Couldn't resolve wildcard address. This is bad. No bmix for you.",
					e);
		}
	}
	int port;
	DatagramSocket socket;
	/**
	 * the host to which we should send heartbeats, or null if we shouldn't send
	 * them.
	 */
	private InetAddress heartBeatDestination;

	/**
	 * the port on the destination to which we should send heartbeats.
	 */
	private final int heartBeatDestPort;

    /**
     * The scaling mode to use when frames received on this input are copied to
     * a layer.
     */
    private final ScaleMode scaleMode;

	/**
	 * The current alpha mode. This can be changed at any time on an existing
	 * instance. Defaults to the extremely documented NATIVE mode.
	 */
	private final AlphaMode alphaMode;

	/**
	 * The transparent colour to use if the alpha mode is CHROMA_KEY.
	 */
	private final Color transparentColour;

	/**
	 * Fully-transparent pixels in the image will appear as this colour instead.
	 * If no translation is desired, this value should be set to null.
	 */
	private final Color shadowColour;

	/**
	 * Default timeout for BLPacketReceivers
	 */
	public static final int DEFAULT_TIMEOUT = 1000;

	/**
	 * Each of these BLPacketSenders will be given the received packet when we
	 * receive it.
	 */
	private final List<BLPacketSender> relaySenders = new ArrayList<BLPacketSender>();
	private final String name;

	private long frameCount = 0;

	/**
	 * If we don't receive a packet from an input for this duration, we should
	 * make the input transparent.
	 */
	private final int timeoutMillis;
	
	/**
     * The offset into the source image to start at. If this value is null,
     * the source image will be scaled and stretched to fit the layer's size;
     * if this value is non-null, the copy will be pixel-for-pixel.
     */
	private Point cropOffset;

	/**
	 * Creates a new BLFrameReceiver for receiving pixel data from a
	 * blinkenlights source.
	 * 
	 * @param name
	 *            the textual name of this input
	 * @param port
	 *            The UDP port number to listen on.
	 * @param address
	 *            The address to listen on (0.0.0.0 or null for all local
	 *            addresses).
	 * @param heartBeatDestination
	 *            the host to which we should send heartbeats, or null if we
	 *            shouldn't send them.
	 * @param heartBeatDestPort
	 *            the port on the proxy host to which we should send the
	 *            heartbeats
	 * @param transparentColour
	 * @param alphaMode
	 * @param scaleMode 
	 * @throws SocketException
	 *             if binding to the specified port and address is not possible.
	 */
	public BLPacketReceiver(String name, int port, InetAddress address,
			InetAddress heartBeatDestination, int heartBeatDestPort,
			ScaleMode scaleMode, AlphaMode alphaMode, Color transparentColour, Color shadowColor,
			int timeoutMillis, Point cropOffset) throws SocketException {
		this.name = name;
		shadowColour = shadowColor;

		this.timeoutMillis = timeoutMillis;
		this.cropOffset = cropOffset;
		if (address == null) {
			address = WILDCARD_ADDRESS;
		}
		this.port = port;
		this.heartBeatDestination = heartBeatDestination;
		this.heartBeatDestPort = heartBeatDestPort;
		if (port < 1) {
			throw new IllegalArgumentException("port must be > 0");
		}
		this.scaleMode = scaleMode;
		this.alphaMode = alphaMode;
		this.transparentColour = transparentColour;

		socket = new DatagramSocket(port, address);
		socket.setSoTimeout(500);
		logger.info("BLFrameReceiver() - port: " + port);
	}

	/**
	 * Releases network resources. Once this method has been called, this
	 * receiver can no longer be used.
	 */
	public void close() {
		logger.info("closing packet receiver: "+getName());
		socket.close();
		logger.info("closing relay "+relaySenders.size()+" senders for: "+getName());
		for (BLPacketSender relay : relaySenders) {
			relay.close();
		}
	}

	/**
	 * Blocks on receive.
	 */
	public BLPacket receive() {
		DatagramPacket packet = new DatagramPacket(buf, buf.length);
		try {
			socket.receive(packet);
			BLPacket parsedPacket = BLPacketFactory.parse(packet, alphaMode,
					transparentColour, shadowColour);
			lastSourceAddr = packet.getAddress().getHostAddress();
			lastPacketReceiveTime = System.currentTimeMillis();
			for (BLPacketSender sender : relaySenders) {
				try {
					sender.send(parsedPacket.getNetworkBytes());
				} catch (Exception e) {
					logger.log(Level.WARNING, "error relaying packet to "
							+ sender, e);
				}
			}
			frameCount++;
			return parsedPacket;
		} catch (SocketTimeoutException e) {
			// this is ok, it's just time to send a heartbeat packet (or do some
			// other interval-based work)
		} catch (Exception e) {
			logger.log(Level.WARNING, "error receiving packet", e);
		}
		return null;
	}

	public void sendHeartBeat() throws IOException {
		if (heartBeatDestination != null) {
			logger.fine("sending heartbeat to: " + heartBeatDestination + ":"
					+ heartBeatDestPort);
			DatagramPacket p = new DatagramPacket(heartBeatBytes,
					heartBeatBytes.length, heartBeatDestination,
					heartBeatDestPort);

			socket.send(p);
		}
	}

	public void addRelaySender(BLPacketSender relaySender) {
		relaySenders.add(relaySender);
	}

	public ScaleMode getScaleMode() {
        return scaleMode;
    }
	
	public AlphaMode getAlphaMode() {
		return alphaMode;
	}

	public Color getTransparentColour() {
		return transparentColour;
	}

	public int getPort() {
		return port;
	}

	public List<BLPacketSender> getRelaySenders() {
		return relaySenders;
	}

	/**
	 * Returns the heartbeat destination address as a String, or null if it
	 * isn't set up.
	 * 
	 * @return the destination heartbeat addr or null
	 */
	public String getHeartBeatDestAddr() {
		if (heartBeatDestination == null)
			return null;
		return heartBeatDestination.getHostAddress();
	}

	public int getHeartBeatDestPort() {
		return heartBeatDestPort;
	}

	public long getLastPacketReceiveTime() {
		return lastPacketReceiveTime;
	}

	public String getLastSourceAddr() {
		return lastSourceAddr;
	}

	public String getName() {
		return name;
	}

	public long getFrameCount() {
		return frameCount;
	}
	
	public int getTimeoutMillis() {
		return timeoutMillis;
	}
	
	public Point getInputCropOffset() {
		return this.cropOffset;
	}
}
