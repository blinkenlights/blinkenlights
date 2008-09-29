/*
 * Created on Sep 28, 2008
 *
 * This code belongs to SQL Power Group Inc.
 */
package de.blinkenlights.bmix.network;

import java.io.Serializable;

/**
 * Simple data structure for holding a network address and port number.
 */
public class HostAndPort implements Serializable {

    private final String addr;
    private final int port;

    public HostAndPort(String addr, int port) {
        this.addr = addr;
        this.port = port;
    }
    
    public String getAddr() {
        return addr;
    }
    
    public int getPort() {
        return port;
    }
    
    @Override
    public String toString() {
        return addr + ":" + port;
    }
    
    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + ((addr == null) ? 0 : addr.hashCode());
        result = prime * result + port;
        return result;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj)
            return true;
        if (obj == null)
            return false;
        if (getClass() != obj.getClass())
            return false;
        HostAndPort other = (HostAndPort) obj;
        if (addr == null) {
            if (other.addr != null)
                return false;
        } else if (!addr.equals(other.addr))
            return false;
        if (port != other.port)
            return false;
        return true;
    }

}
