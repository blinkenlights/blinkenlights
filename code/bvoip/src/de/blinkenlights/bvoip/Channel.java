package de.blinkenlights.bvoip;

import de.blinkenlights.bvoip.asterisk.AGISession;
import de.blinkenlights.bvoip.blt.BLTClient;

/**
 * A channel in the system. Once the channel has closed, it will be dead and
 * will be removed from the channel list.
 */
public class Channel {

	private final int channelNum;
	private AGISession agiSession;
	private BLTClient client;
	private final ChannelList parentList;
	private boolean closed = false;
	
	public Channel(ChannelList parentList, int channelNum) {
		this.parentList = parentList;
		this.channelNum = channelNum;
	}

	public BLTClient getClient() {
		return client;
	}

	public void setClient(BLTClient client) {
		this.client = client;
	}

	public int getChannelNum() {
		return channelNum;
	}

	public void setAgiSession(AGISession agiSession) {
		this.agiSession = agiSession;
	}
	
	public AGISession getAgiSession() {
		return agiSession;
	}

	public void close() {
		closed = true;
		parentList.channelClosing(this);
	}
	
	public boolean isClosed() {
		return closed;
	}
}
