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

import java.awt.image.BufferedImage;
import java.io.IOException;
import java.net.InetAddress;

import de.blinkenlights.bmix.network.BLNetworkException;
import de.blinkenlights.bmix.network.BLPacketSender;
import de.blinkenlights.bmix.protocol.BLFramePacket;

/**
 * A client to the BMix server.  Sends heartbeats at regular intervals,
 * and sends frame packets.
 */
class BMixClient implements FrameTarget {

    private final InetAddress host;
    private final int port;

    private final BLPacketSender sender;
    
    public BMixClient(InetAddress host, int port) throws BLNetworkException {
        this.host = host;
        this.port = port;
        sender = new BLPacketSender(host.getHostAddress(), port);
    }

    public void start() {

    }
    
    public void stop() {

    }

    public void putFrame(BufferedImage image) throws IOException {
        BLFramePacket packet = new BLFramePacket(image);
        sender.send(packet.getNetworkBytes());
    }

    public void gameEnding() {
        // TODO Auto-generated method stub
        
    }

	public InetAddress getHost() {
		return host;
	}

	public int getPort() {
		return port;
	}
}
