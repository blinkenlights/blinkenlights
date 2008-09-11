package de.blinkenlights.bvoip;

import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;

import de.blinkenlights.bvoip.asterisk.AGISession;
import de.blinkenlights.bvoip.blt.BLTClientManager;

/**
 * A channel in the system. Once the channel has closed, it will be dead and
 * will be removed from the channel list.
 */
public class Channel {

	private final int channelNum;
	private AGISession agiSession;
	private BLTClientManager client;
	private final ChannelList parentList;
	private boolean closed = false;
	
	private final PropertyChangeSupport pcs = new PropertyChangeSupport(this);
	
	public Channel(ChannelList parentList, int channelNum) {
		this.parentList = parentList;
		this.channelNum = channelNum;
	}

	public BLTClientManager getClient() {
		return client;
	}

	public void setClient(BLTClientManager client) {
		this.client = client;
	}

	public int getChannelNum() {
		return channelNum;
	}

	public void setAgiSession(AGISession agiSession) {
		AGISession oldSession = this.agiSession;
		this.agiSession = agiSession;
		pcs.firePropertyChange("agiSession", oldSession, agiSession);
	}
	
	public AGISession getAgiSession() {
		return agiSession;
	}

	public void close() {
		closed = true;
		setAgiSession(null);
		parentList.channelClosing(this); // TODO: if setting it to closed fired an event, we wouldn't need this at all.
	}
	
	public boolean isClosed() {
		return closed;
	}

	public void addPropertyChangeListener(PropertyChangeListener listener) {
		pcs.addPropertyChangeListener(listener);
	}

	public PropertyChangeListener[] getPropertyChangeListeners() {
		return pcs.getPropertyChangeListeners();
	}

	public void removePropertyChangeListener(PropertyChangeListener listener) {
		pcs.removePropertyChangeListener(listener);
	}

	public boolean isAvailable() {
		return agiSession == null;
	}
	
	
}
