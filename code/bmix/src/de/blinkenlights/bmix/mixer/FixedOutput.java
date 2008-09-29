package de.blinkenlights.bmix.mixer;

import java.io.IOException;
import java.util.Collections;
import java.util.List;
import java.util.logging.Logger;

import de.blinkenlights.bmix.network.BLPacketSender;
import de.blinkenlights.bmix.network.HostAndPort;

/**
 * This class is an output which gets pixels from a source image and sends
 * BLFramePackets to a destination.
 */
public class FixedOutput extends AbstractOutput {
    private static final Logger logger = Logger.getLogger(FixedOutput.class.getName());
    
    private final BLPacketSender sender;

    /**
     * The last time a packet was sent. Dropped output packets are not counted.
     */
    private long lastSendTime;

    /**
     * Creates a new output definition with the given sender and viewport. You
     * specify the image source every time you want to send something.
     * 
     * @param sender
     *            The packet sender to send data to.
     * @param source
     *            The image to send a portion of. Normally this will be the
     *            virtual buffer, but if you're making novel use of the API, it
     *            can be any BLImage.
     */
    public FixedOutput(BLPacketSender sender, BLImage source, long minSendInterval,
    		PacketType packetType) {
        super(source, packetType, minSendInterval);
        this.sender = sender;
    }

    /* (non-Javadoc)
     * @see de.blinkenlights.bmix.mixer.OutputXX#send()
     */
    public void send() throws IOException {
        boolean sent = sendSingleFrame(sender, lastSendTime, logger);
        if (sent) {
            lastSendTime = System.currentTimeMillis();
        }
    }
        
	public List<HostAndPort> getDestinations() {
	    return Collections.singletonList(new HostAndPort(sender.getAddress(), sender.getPort()));
	}
	
	public long getLastSendTime() {
	    return lastSendTime;
	}
	
	@Override
	public String toString() {
	    return "" + sender + " " + viewports;
	}
}
