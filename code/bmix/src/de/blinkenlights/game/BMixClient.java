/*
 * Created on Sep 25, 2008
 */
package de.blinkenlights.game;

import java.awt.Image;
import java.net.InetAddress;

/**
 * A client to the BMix server.  Sends heartbeats at regular intervals,
 * and sends frame packets.
 */
class BMixClient implements FrameTarget {

    private final InetAddress host;
    private final int port;

    public BMixClient(InetAddress host, int port) {
        this.host = host;
        this.port = port;
    }

    public void start() {
        // TODO Auto-generated method stub

    }
    
    public void stop() {
        // TODO Auto-generated method stub

    }

    public void putFrame(Image image) {
        // TODO Auto-generated method stub
        
    }
}
