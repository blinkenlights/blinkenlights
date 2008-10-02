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
