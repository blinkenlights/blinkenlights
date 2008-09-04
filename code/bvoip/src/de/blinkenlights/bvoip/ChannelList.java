package de.blinkenlights.bvoip;


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
	
	public ChannelList(int nchannels) {
		channels = new Channel[nchannels];
	}
	
	public synchronized Channel acquire() {
		for (int i = 0; i < channels.length; i++) {
			if (channels[i] == null) {
				Channel c = new Channel(this, i);
				channels[i] = c;
				// TODO fire a connection event
				return c;
			}
		}
		return null; // throw exception?
	}
	
	synchronized void channelClosing(Channel c) {
		if (channels[c.getChannelNum()] != c) {
			throw new IllegalStateException("The closing channel isn't at the index it claims to be at, Winston.");
		}
		channels[c.getChannelNum()] = null;
		// TODO fire channel closed event
	}
}
