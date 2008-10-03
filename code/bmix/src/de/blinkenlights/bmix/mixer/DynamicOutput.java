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
package de.blinkenlights.bmix.mixer;

import java.io.IOException;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;

import de.blinkenlights.bmix.network.BLNetworkException;
import de.blinkenlights.bmix.network.BLPacketReceiver;
import de.blinkenlights.bmix.network.BLPacketSender;
import de.blinkenlights.bmix.network.HostAndPort;
import de.blinkenlights.bmix.network.BLPacketReceiver.AlphaMode;
import de.blinkenlights.bmix.protocol.BLHeartbeatPacket;
import de.blinkenlights.bmix.protocol.BLPacket;

public class DynamicOutput extends AbstractOutput {

    private static final Logger logger = Logger.getLogger(DynamicOutput.class.getName());
    
    private static class DestInfo {
        
        long lastHeartbeat;
        long lastSend;
        final BLPacketSender sender;
        
        public DestInfo(BLPacketSender sender) {
            super();
            this.sender = sender;
        }
        
    }
    
    private final Map<HostAndPort, DestInfo> destinations =
        Collections.synchronizedMap(new HashMap<HostAndPort, DestInfo>());
    
    private final long heartbeatTimeout;

    private long overallLastSendTime;

    private HeartbeatReceiverTask receiverTask;

    private final String listenAddr;

    private final int listenPort;
    
    public DynamicOutput(String listenAddr, int listenPort, BLImage source,
            long minInterval, PacketType packetFormat, long heartbeatTimeout)
    throws SocketException, UnknownHostException {
        
        super(source, packetFormat, minInterval);
        this.listenAddr = listenAddr;
        this.listenPort = listenPort;
        this.heartbeatTimeout = heartbeatTimeout;
        receiverTask = new HeartbeatReceiverTask(listenPort, listenAddr);
    }

    public synchronized void close() {
        receiverTask.close();
        for (Map.Entry<HostAndPort, DestInfo> entry : destinations.entrySet()) {
            entry.getValue().sender.close();
        }
        destinations.clear();
    }
    
    public List<HostAndPort> getDestinations() {
        return new ArrayList<HostAndPort>(destinations.keySet());
    }

    private class HeartbeatReceiverTask implements Runnable {

        private final BLPacketReceiver heartbeatReceiver;
        
        private boolean stopRequested = false;

        HeartbeatReceiverTask(int listenPort, String listenAddr) throws SocketException, UnknownHostException {
            logger.fine("creating heartbeat receiver task bound to "+listenAddr);
            heartbeatReceiver = new BLPacketReceiver(
                    "Heartbeat receiver", listenPort, InetAddress.getByName(listenAddr),
                    null, 0, AlphaMode.OPAQUE, null, null, BLPacketReceiver.DEFAULT_TIMEOUT, null);
        }
        
        public void run() {
        	try {
	            for (;;) {
	                BLPacket bp;
	                synchronized (this) {
	                    if (isStopRequested()) break;                    
	                    bp = heartbeatReceiver.receive();
	                }
	                logger.finest("Got packet: " + bp);
	                if (bp instanceof BLHeartbeatPacket) {
	                    BLHeartbeatPacket hbp = (BLHeartbeatPacket) bp;
	                    HostAndPort hap = new HostAndPort(hbp.getFromHost(), hbp.getFromPort());
	                    
	                    try {
	                        DestInfo destInfo = destinations.get(hap);
	                        if (destInfo == null) {
	                            destInfo = new DestInfo(new BLPacketSender(hap.getAddr(), hap.getPort()));
	                            destinations.put(hap, destInfo);
	                            logger.fine("Added new destination host " + hap);
	                        } else {
	                            long delta = System.currentTimeMillis() - destInfo.lastHeartbeat;
	                            logger.fine("Got heartbeat from " + hap + " (" + delta + "ms since last heartbeat)");
	                        }
	                        destInfo.lastHeartbeat = System.currentTimeMillis();
	                    } catch (BLNetworkException ex) {
	                        logger.log(Level.INFO, "Not adding specially malformed host to destinations list: " + hap, ex);
	                    }
	                }
	            }
        	} finally {
                heartbeatReceiver.close();
        	}
        }

        public synchronized boolean isStopRequested() {
            return stopRequested;
        }
        
        public synchronized void close() {
            stopRequested = true;
        }
    }
        
    public void send() throws IOException {
        Iterator<Map.Entry<HostAndPort, DestInfo>> destIt = destinations.entrySet().iterator();
        long now = System.currentTimeMillis();
        while (destIt.hasNext()) {
            Map.Entry<HostAndPort, DestInfo> entry = destIt.next();
            HostAndPort hap = entry.getKey();
            DestInfo destInfo = entry.getValue();
            
            long delta = now - destInfo.lastHeartbeat;
            if (delta > heartbeatTimeout) {
                logger.fine("Removing destination " + hap + " (no heartbeat for " + delta + " ms)");
                destIt.remove();
                destInfo.sender.close();
            } else {
                sendSingleFrame(destInfo.sender, destInfo.lastSend, logger);
                destInfo.lastSend = now;
            }
        }
        overallLastSendTime = now;
    }

    public long getLastSendTime() {
        return overallLastSendTime;
    }

    public void start() {
        new Thread(receiverTask, "Dynamic output@" + listenAddr+":"+listenPort).start();
    }
    
}
