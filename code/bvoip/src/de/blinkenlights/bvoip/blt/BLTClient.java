package de.blinkenlights.bvoip.blt;

import java.net.InetAddress;
import java.util.logging.Logger;


/**
 * This class represents an active blinkenlights telephony client (ie, blccc)
 * @author dfraser
 *
 */
public class BLTClient {
	
	private static final Logger logger = Logger.getLogger(BLTClient.class.getName());
	
	private final InetAddress address;
	private final int sourcePort;
	private long lastHeartBeatTime;

	private final int destPort;

	public BLTClient(ClientIdentifier ci, int destPort) {
		this.destPort = destPort;
		this.address = ci.getAddress();
		this.sourcePort = ci.getPort();
		this.lastHeartBeatTime = System.currentTimeMillis();
	}
	
	public void heartBeat() {
		lastHeartBeatTime = System.currentTimeMillis();
	}
	
	public long getLastHeartBeatTime() {
		return lastHeartBeatTime;
	}
	
	public int getDestPort() {
		return destPort;
	}
	
	public InetAddress getAddress() {
		return address;
	}

	@Override
	public String toString() {
		return address.toString()+":"+sourcePort+ "(destport "+destPort+")";
	}
}
