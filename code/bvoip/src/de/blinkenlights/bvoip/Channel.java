package de.blinkenlights.bvoip;

import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;

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
	
	private final PropertyChangeSupport pcs = new PropertyChangeSupport(this);
	
	public Channel(ChannelList parentList, int channelNum) {
		this.parentList = parentList;
		this.channelNum = channelNum;
	}

	public synchronized BLTClient getClient() {
		return client;
	}

	public synchronized void setClient(BLTClient client) {
		BLTClient oldClient = this.client;
		this.client = client;
		pcs.firePropertyChange("client", oldClient, client);
	}

	public int getChannelNum() {
		return channelNum;
	}

	public synchronized void setAgiSession(AGISession agiSession) {
		AGISession oldSession = this.agiSession;
		this.agiSession = agiSession;
		pcs.firePropertyChange("agiSession", oldSession, agiSession);
	}
	
	public synchronized AGISession getAgiSession() {
		return agiSession;
	}

	public void close() {
		closed = true; // TODO: fire event
		setAgiSession(null);
		setClient(null);
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

	public synchronized boolean isAvailable() {
		return agiSession == null;
	}

	/**
	 * Returns true if this channel has both an AGI session and a BL Client.
	 */
	public synchronized boolean isConnected() {
		return agiSession != null && client != null;
	}
	
	
}
