package de.blinkenlights.bmix.mixer;

import java.awt.Rectangle;
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
    private final BLImageViewport viewport;
    private final PacketType packetType;
    private final int multiframeBpp;
    
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
    public Output(BLPacketSender sender, BLImage source, Rectangle viewport, long minSendInterval,
    		PacketType packetType, int multiframeBpp) {
        super();
        this.sender = sender;
        this.minSendInterval = minSendInterval;
        this.viewport = new BLImageViewport(source, viewport);
        this.packetType = packetType;
        this.multiframeBpp = multiframeBpp;
        if(packetType == PacketType.MCU_MULTIFRAME && multiframeBpp != 4 && multiframeBpp != 8) {
        	throw new IllegalArgumentException("multiframeBpp is invalid: " + multiframeBpp + " (must be 4 or 8)");
        }
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
            	AbstractFramePacket p = new BLFramePacket(viewport);
                sender.send(p.getNetworkBytes());            	
            } else if(packetType == PacketType.MCU_MULTIFRAME) {
            	AbstractFramePacket p = new BLMultiframePacket(viewport, multiframeBpp);
                sender.send(p.getNetworkBytes());
            } else {
            	throw new AssertionError("Unsupported packet type: " + packetType.name());
            }
        }
    }
    
    @Override
    public String toString() {
        return "" + sender + " " + viewport;
    }
}
