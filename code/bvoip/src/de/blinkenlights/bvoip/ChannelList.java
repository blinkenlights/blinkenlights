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
	
	public boolean isChannelActive(int channelNum) {
		if (channelNum < 0 || channelNum >= channels.length) {
			throw new IllegalArgumentException("channel is out of bounds, should be between 0 and "+(channels.length-1)+" but was "+channelNum);
		}
		return !channels[channelNum].isAvailable();
	}
	
	synchronized void channelClosing(Channel c) {
		if (channels[c.getChannelNum()] != c) {
			throw new IllegalStateException("The closing channel isn't at the index it claims to be at, Winston.");
		}
		channels[c.getChannelNum()].close();
	}

	public List<Channel> getChannels() {
		return Arrays.asList(channels);
	}
}
