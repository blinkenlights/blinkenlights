/*
 * Created on Sep 25, 2008
 */
package de.blinkenlights.game;

import java.net.InetAddress;

/**
 * A client to the Blinkenlights Telephony server.
 */
class BLTClient implements UserInputSource {

    private final InetAddress host;
    private final int port;
    private final String did;

    public BLTClient(InetAddress host, int port, String did) {
        this.host = host;
        this.port = port;
        this.did = did;
    }

    public void start() {
        // TODO Auto-generated method stub

    }
    
    public void stop() {
        // TODO Auto-generated method stub

    }

    public Character getKeystroke() {
        // TODO Auto-generated method stub
        return null;
    }

    public void waitForGameStart() {
        // TODO Auto-generated method stub
        
    }

	public InetAddress getHost() {
		return host;
	}

	public int getPort() {
		return port;
	}

	public String getDid() {
		return did;
	}
}
