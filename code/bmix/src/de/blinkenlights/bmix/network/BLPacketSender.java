package de.blinkenlights.bmix.network;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.logging.Logger;

/**
 * This class is a packet sender that sends packets over the network. The
 * raw bytes to send must be passed in from a class that knows how to make them.
 */
public class BLPacketSender {
    DatagramSocket socket = null;
	InetAddress address = null;
	int port;
	
	private final static Logger logger = Logger.getLogger(BLPacketSender.class.getName());
	
	
	/**
	 * Creates a new BLPacketSender.
	 * 
	 * @param host the hostname or IP address of the destination
	 * @param port the port on the destination
	 */
	public BLPacketSender(String host, int port) throws BLNetworkException {
		try {
			address = InetAddress.getByName(host);
			this.port = port;
			socket = new DatagramSocket();
		} catch (UnknownHostException e) {
			throw new BLNetworkException(e.getMessage());
		} catch (SocketException e) {
			throw new BLNetworkException(e.getMessage());
		}
		logger.info("BLPacketSender() - host: " + host + " - port: " + port);		
	}

	
	/**
	 * Sends a message to the client.
	 * 
	 * @param buf the buffer to send
	 * @throws BLNetworkException if there was an error sending the packet
	 */
	public void send(byte buf[]) throws BLNetworkException {
		send(buf, 0, buf.length);
	}

	
	/**
	 * Sends a message to the client.
	 * 
	 *  @param buf the buffer to send
	 *  @param offset the offset in the buffer
	 *  @param length the length in the buffer
	 */
	public void send(byte buf[], int offset, int length) throws BLNetworkException {
		if(buf.length < 1) {
			throw new BLNetworkException("buffer length must be > 0");
		}
		if(offset < 0 || length < 1 || (offset + length) > buf.length) {
			throw new BLNetworkException("send parameters are invalid - offset: " + offset + 
					" - length: " + length + " - buf len: " + buf.length);
		}
        DatagramPacket packet = new DatagramPacket(buf, offset, buf.length, address, port);
        try {
			socket.send(packet);
		} catch (IOException e) {
			throw new BLNetworkException("error sending packet of len: " + buf.length + 
					" to host: " + packet.getAddress() + "  - port: " + port);
		}		
	}
	
	@Override
	public String toString() {
	    return address.getHostAddress() + ":" + port;
	}
}
