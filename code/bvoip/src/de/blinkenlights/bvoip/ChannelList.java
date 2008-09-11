package de.blinkenlights.bvoip;

import java.util.Arrays;
import java.util.List;


/**
 * The BLCCC phone protocol expects there to be a fixed number of telephone
 * channels (lines) available. This class perpetuates that illusion.
 * <p>
 * Instances of this class are thread safe because the channel list represents
 * the shared state between Asterisk land and BLCCC land, each of which run in
 * separate threads(es).
 */
public final class ChannelList {

	private final Channel[] channels;
		
	public ChannelList(int nchannels) {
		channels = new Channel[nchannels];
		
		// this should be done last since we're giving out references
		for (int i = 0; i < nchannels; i++) {
			channels[i] = new Channel(this, i);
		}
	}
	

	
	public synchronized Channel acquire() {
		for (int i = 0; i < channels.length; i++) {
			if (channels[i].isAvailable()) {
				return channels[i];
			}
		}
		return null; // throw exception?
	}
	
	public int getNumChannels() {
		return channels.length;
	}

	public List<Channel> getChannels() {
		return Arrays.asList(channels);
	}
}
