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

    public void gameEnding() {
        // TODO Auto-generated method stub
        
    }

    public boolean isUserPresent() {
        // TODO Auto-generated method stub
        return false;
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
	
	private class Pinger implements Runnable {

		private long pingIntervalMillis = 15000;
		private boolean stopRequested = false;

		public void run() {
			
			while (!isStopRequested()) {
				
				
				
				try {
					Thread.sleep(pingIntervalMillis);
				} catch (InterruptedException e) {
					// just do the loop again
				}
			}
		}

		private synchronized boolean isStopRequested() {
			return stopRequested ;
		}
		
		public synchronized void setStopRequested(boolean stopRequested) {
			this.stopRequested = stopRequested;
		}
		
	}
}
