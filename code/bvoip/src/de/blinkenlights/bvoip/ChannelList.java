package de.blinkenlights.bvoip;

import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;


/**
 * The BLCCC phone protocol expects there to be a fixed number of telephone
 * channels (lines) available. This class perpetuates that illusion.
 * <p>
 * Instances of this class are thread safe because the channel list represents
 * the shared state between Asterisk land and BLCCC land, each of which run in
 * separate threads(es).
 */
public class ChannelList {

	private final Channel[] channels;
	
	Set<ConnectionEventListener> connectionEventListeners = new HashSet<ConnectionEventListener>();
	
	public ChannelList(int nchannels) {
		channels = new Channel[nchannels];
	}
	
	public void addConnectionEventListener(ConnectionEventListener listener) {
		if (listener != null) {
			connectionEventListeners.add(listener);
		}
	}
	
	public void removeConnectionEventListener(ConnectionEventListener listener) {
		if (listener != null) {
			connectionEventListeners.remove(listener);
		}
	}
	
	public synchronized Channel acquire() {
		for (int i = 0; i < channels.length; i++) {
			if (channels[i] == null) {
				Channel c = new Channel(this, i);
				channels[i] = c;
				
				// TODO: this is the wrong place to fire an event, the agisession isn't set up yet!!
				ConnectionEvent e = new ConnectionEvent(c);
				for (Iterator<ConnectionEventListener> listenerIter = connectionEventListeners.iterator(); listenerIter
						.hasNext();) {
					ConnectionEventListener listener = listenerIter.next();
					listener.channelConnected(e);
				}
				
				return c;
			}
		}
		return null; // throw exception?
	}
	
	public int getNumChannels() {
		return channels.length;
	}
	
	public boolean isChannelActive(int channelNum) {
		if (channelNum < 0 || channelNum >= channels.length) {
			throw new IllegalArgumentException("channel is out of bounds, should be between 0 and "+(channels.length-1)+" but was "+channelNum);
		}
		return channels[channelNum] != null;
	}
	
	synchronized void channelClosing(Channel c) {
		if (channels[c.getChannelNum()] != c) {
			throw new IllegalStateException("The closing channel isn't at the index it claims to be at, Winston.");
		}
		channels[c.getChannelNum()] = null;
		// TODO fire channel closed event
	}
}
