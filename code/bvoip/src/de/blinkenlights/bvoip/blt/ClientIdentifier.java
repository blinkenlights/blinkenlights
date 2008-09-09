package de.blinkenlights.bvoip.blt;

import java.net.InetAddress;

public class ClientIdentifier {

	private final InetAddress address;
	private final int port;

	public ClientIdentifier(InetAddress addr, int port) {
		this.address = addr;
		this.port = port;
	}
	
	@Override
	public String toString() {
		return address.toString()+":"+port;
	}
	
	@Override
	public boolean equals(Object o) {
		if (o != null && o instanceof ClientIdentifier) {
			ClientIdentifier ci = (ClientIdentifier) o;
			if (ci.address.equals(address) && ci.port == port) {
				return true;
			}
		}
		return false;
	}
	
	@Override
	public int hashCode() {
		return (address.hashCode() << 16) | port;
	}

	public InetAddress getAddress() {
		return address;
	}
	
	public int getPort() {
		return port;
	}
}
