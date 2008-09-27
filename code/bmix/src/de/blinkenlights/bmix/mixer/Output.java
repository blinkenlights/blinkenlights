package de.blinkenlights.bmix.mixer;

import java.awt.Rectangle;
import java.util.ArrayList;
import java.util.List;
import java.util.logging.Logger;

import de.blinkenlights.bmix.network.BLNetworkException;
import de.blinkenlights.bmix.network.BLPacketSender;
import de.blinkenlights.bmix.protocol.AbstractFramePacket;
import de.blinkenlights.bmix.protocol.BLFramePacket;
import de.blinkenlights.bmix.protocol.BLMultiframePacket;

/**
 * This class is an output which gets pixels from a source image and sends
 * BLFramePackets to a destination.
 */
public class Output {
    public enum PacketType { MCU_FRAME, MCU_MULTIFRAME };
	
    private static final Logger logger = Logger.getLogger(Output.class.getName());
    private final BLPacketSender sender;
    private final PacketType packetType;
    
    /**
     * All screens for this output. Screen IDs are the list indices.
     */
    private final List<BLImageViewport> viewports = new ArrayList<BLImageViewport>();
    
    /**
     * The minimum amount of time between packets. All attempts to send a packet
     * less than this amount of time after the previously-sent packet will
     * result in no output.
     */
    private final long minSendInterval;
    
    /**
     * The last time a packet was sent. Dropped output packets are not counted.
     */
    private long lastSendTime;

    /**
     * The image to send a portion of. Viewport geometry is in terms of this
     * image. Normally this will be the virtual buffer, but under novel use of
     * the API, it could be any BLImage.
     */
    private final BLImage source;

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
     * @param viewport
     *            The region of the image that will be cropped out and sent.
     */
    public Output(BLPacketSender sender, BLImage source, long minSendInterval,
    		PacketType packetType) {
        super();
        this.sender = sender;
        this.source = source;
        this.minSendInterval = minSendInterval;
        this.packetType = packetType;
    }

    /**
     * Reads the data in this output's viewport rectangle from the given BLImage
     * and sends it via this output's BLPacketSender.
     * 
     * @throws BLNetworkException if there was an error sending the packet
     */
    public void send() throws BLNetworkException {
        long now = System.currentTimeMillis();
        if (lastSendTime + minSendInterval > now) {
            logger.fine("Suppressing output packet because it came " +
                    (lastSendTime + minSendInterval - now) + "ms too soon");
        } else {
            lastSendTime = now;
            if(packetType == PacketType.MCU_FRAME) {
            	AbstractFramePacket p = new BLFramePacket(viewports.get(0));
                sender.send(p.getNetworkBytes());            	
            } else if(packetType == PacketType.MCU_MULTIFRAME) {
            	BLMultiframePacket p = new BLMultiframePacket(viewports);
                sender.send(p.getNetworkBytes());
            } else {
            	throw new AssertionError("Unsupported packet type: " + packetType.name());
            }
        }
    }
    
    @Override
    public String toString() {
        return "" + sender + " " + viewports;
    }

    /**
     * Adds a new screen to this output. Only the MCU_MULTIFRAME output
     * type supports more than one screen.
     * @param bounds
     * @param bpp bits per pixel. Ignored unless this is an MCU_MULTIFRAME output.
     */
    public void addScreen(Rectangle bounds, int bpp) {
        if (viewports.size() > 0 && packetType != PacketType.MCU_MULTIFRAME) {
            throw new UnsupportedOperationException(
                    "Packet type " + packetType + " does not support multiple screens");
        }
        viewports.add(new BLImageViewport(source, bounds, bpp));
    }
    
	public List<BLImageViewport> getViewports() {
		return viewports;
	}

	public PacketType getPacketType() {
		return packetType;
	}
 
	public long getMinSendInterval() {
		return minSendInterval;
	}

	public long getLastSendTime() {
		return lastSendTime;
	}
    
    public String getDestAddr() {
    	return sender.getAddress();
    }
    
    public int getDestPort() {
    	return sender.getPort();
    }
}
