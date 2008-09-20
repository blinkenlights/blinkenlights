package de.blinkenlights.bmix.statistics;

import java.awt.Rectangle;

public class OutputStatistics {
	private final Rectangle viewport;
	private final String destAddr;
	private final int destPort;
	private final long minSendInterval;
	private final String name;
	private final int multiframeBpp;

	public OutputStatistics(Rectangle viewport, String destAddr, int destPort,
			long minSendInterval, String name, int multiframeBpp) {
				this.viewport = viewport;
				this.destAddr = destAddr;
				this.destPort = destPort;
				this.minSendInterval = minSendInterval;
				this.name = name;
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

	public String getName() {
		return name;
	}

	public int getMultiframeBpp() {
		return multiframeBpp;
	}
}
