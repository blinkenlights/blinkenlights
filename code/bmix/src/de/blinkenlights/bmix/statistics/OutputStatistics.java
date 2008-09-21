package de.blinkenlights.bmix.statistics;

import java.awt.Rectangle;

import de.blinkenlights.bmix.mixer.Output.PacketType;

public class OutputStatistics implements StatisticsItem {

	private static final long serialVersionUID = -982303235686899928L;
	
	/**
	 * The unique ID of the object these stats are about.
	 */
	private final long id;

	private final Rectangle viewport;
	private final String destAddr;
	private final int destPort;
	private final long minSendInterval;
	private final PacketType packetType;
	private final int multiframeBpp;


	public OutputStatistics(long id, Rectangle viewport, String destAddr, int destPort,
			long minSendInterval, PacketType packetType, int multiframeBpp) {
				this.id = id;
				this.viewport = viewport;
				this.destAddr = destAddr;
				this.destPort = destPort;
				this.minSendInterval = minSendInterval;
				this.packetType = packetType;
				this.multiframeBpp = multiframeBpp;
	}
	
	public long getId() {
		return id;
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

	public PacketType getPacketType() {
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

	public String getName() {
		return "Output to " + destAddr + ":" + destPort;
	}

	public String toHtml() {
	    String packetTypeDesc = packetType.name();
	    if (packetType == PacketType.MCU_FRAME) {
	        packetTypeDesc += " (maxval 255)";
	    } else if (packetType == PacketType.MCU_MULTIFRAME) {
	        packetTypeDesc += " (" + multiframeBpp + " bpp)";
	    }
        return String.format(
                "<html><table cellpadding=1 cellspacing=0>" +
                "<tr><th colspan=2>Output %s:%d" +
                "<tr><td>Viewport<td>%dx%d+%d+%d" +
                "<tr><td>Min frame interval<td>%dms" +
                "<tr><td>Packet Type<td>%s",
                destAddr, destPort,
                viewport.width, viewport.height, viewport.x, viewport.y,
                minSendInterval,
                packetTypeDesc);
	}
}
