package de.blinkenlights.bmix.statistics;

import java.awt.Rectangle;

public class OutputStatistics {
	private final Rectangle viewport;
	private final String destAddr;
	private final int destPort;
	private final long minSendInterval;
	private final String packetType;
	private final int multiframeBpp;

	public OutputStatistics(Rectangle viewport, String destAddr, int destPort,
			long minSendInterval, String packetType, int multiframeBpp) {
				this.viewport = viewport;
				this.destAddr = destAddr;
				this.destPort = destPort;
				this.minSendInterval = minSendInterval;
				this.packetType = packetType;
				this.multiframeBpp = multiframeBpp;
	}

	public Rectangle getViewport() {
		return viewport;
	}

	public String getDestAddr() {
		return destAddr;
	}

	public int getDestPort() {
		return destPort;
	}

	public long getMinSendInterval() {
		return minSendInterval;
	}

	public String getPacketType() {
		return packetType;
	}
	
	public int getMultiframeBpp() {
		return multiframeBpp;
	}
	
	public String toString() {
		StringBuilder str = new StringBuilder();
		str.append("Output - Dest Addr: " + destAddr + " - Dest Port: " + destPort + "\n");
		str.append("  Viewport - x: " + viewport.x + " - y: " + viewport.y + 
				" - w: " + viewport.width + " - h: " + viewport.height + "\n"); 
		str.append("  Minimum Send Interval: " + minSendInterval + "\n");
		str.append("  Packet Type: " + packetType + "\n");
		str.append("  Multiframe bpp: " + multiframeBpp + "\n");
		str.append("\n");
		return str.toString();
	}
}
