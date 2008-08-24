package de.blinkenlights.bmix.network;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
import java.util.logging.Level;
import java.util.logging.Logger;

import de.blinkenlights.bmix.protocol.BLFramePacket;
import de.blinkenlights.bmix.protocol.BLHeartbeatPacket;
import de.blinkenlights.bmix.protocol.BLPacket;
import de.blinkenlights.bmix.protocol.BLPacketException;
import de.blinkenlights.bmix.protocol.BLPacketFactory;

/**
 * This class is a BL packet receiver. It listens on a port and waits for incoming
 * packets. When a packet is received it returns a BLPacket of the correct type or
 * null if there was an error.
 */
public class BLPacketReceiver {
	
	private final static Logger logger = Logger.getLogger(BLPacketReceiver.class.getName());
	
	private static final int HEARTBEAT_PORT = 4242;
	byte buf[] = new byte[65536];
    private final byte[] heartBeatBytes = new BLHeartbeatPacket(BLHeartbeatPacket.VERSION_NUMBER).getNetworkBytes();
    
    /**
     * The default address to listen on (binds to all local addresses).
     */
    private static final InetAddress WILDCARD_ADDRESS;
    
    static {
        try {
            WILDCARD_ADDRESS = InetAddress.getByName("0.0.0.0");
        } catch (UnknownHostException e) {
            throw new RuntimeException("Couldn't resolve wildcard address. This is bad. No bmix for you.", e);
        }
    }
	int port;
	DatagramSocket socket;
	/**
	 *  the host to which we should send heartbeats, or null if we shouldn't send them.
	 */
	private InetAddress heartBeatDestination;
	
	
	
	/**
	 * Creates a new BLFrameReceiver for receiving pixel data from a blinkenlights source.
	 * 
	 * @param port The UDP port number to listen on.
	 * @param address The address to listen on (0.0.0.0 or null for all local addresses).
	 * @param heartBeatDestination the host to which we should send heartbeats, or null if we shouldn't send them.
	 * @throws SocketException if binding to the specified port and address is not possible.
	 */
	public BLPacketReceiver(int port, InetAddress address, InetAddress heartBeatDestination) throws SocketException  {
		if (address == null) {
			address = WILDCARD_ADDRESS;
		}
		this.port = port;
		this.heartBeatDestination = heartBeatDestination;
		if(port < 1) {
			throw new IllegalArgumentException("port must be > 0");
		}		
		socket = new DatagramSocket(port, address);
		socket.setSoTimeout(1000);
		logger.info("BLFrameReceiver() - port: " + port);
	}
	
	/**
	 * Blocks on receive. 
	 */
	public BLPacket receive() {
		DatagramPacket packet = new DatagramPacket(buf, buf.length);
		try {
			socket.receive(packet);
			return BLPacketFactory.parse(buf, packet.getLength());
		} catch (SocketTimeoutException e) {
			// this is ok, it's just time to send a heartbeat packet (or do some other interval-based work)
		} catch (IOException e) {
			logger.log(Level.WARNING,"error receiving packet",e);
		} catch (BLPacketException e) {
			logger.log(Level.WARNING,"error parsing received packet",e);
		}
		return null;
	}
	
	/**
	 * Main for testing.
	 * 
	 * @param args command-line arguments
	 * 
	 * Arguments:
	 * 
	 *  - port		the port number
	 */
	public static void main(String args[]) throws Exception {
		if(args.length < 1) {
			logger.severe("listening port number must be supplied");
			return;
		}
		int port = 0;
		try {
			port = Integer.parseInt(args[0]);
		} catch(NumberFormatException e) {
			logger.log(Level.SEVERE,args[0]+" wasn't something that I could parse as a number!",e);
			return;
		}
		BLPacketReceiver blfr = new BLPacketReceiver(port,null,null);
		while(true) {
			BLPacket blp = blfr.receive();
			if(blp instanceof BLFramePacket) {
				BLFramePacket blfp = (BLFramePacket)blp;
				logger.fine(blfp.toString());
			}
		}
	}
	
	public void sendHeartBeat() throws IOException {
		if (heartBeatDestination != null) {
			logger.fine("sending heartbeat to: "+heartBeatDestination);
			DatagramPacket p = new DatagramPacket(heartBeatBytes, heartBeatBytes.length, heartBeatDestination, HEARTBEAT_PORT);
			socket.send(p);
		}
	}
}
