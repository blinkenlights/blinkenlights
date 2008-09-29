/*
 * Created on Sep 28, 2008
 */
package de.blinkenlights.bmix.mixer;

import java.awt.Rectangle;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.logging.Logger;

import de.blinkenlights.bmix.network.BLPacketSender;
import de.blinkenlights.bmix.protocol.AbstractFramePacket;
import de.blinkenlights.bmix.protocol.BLFramePacket;
import de.blinkenlights.bmix.protocol.BLMultiframePacket;

public abstract class AbstractOutput implements Output {

    public static enum PacketType { MCU_FRAME, MCU_MULTIFRAME }

    protected final PacketType packetType;
    
    /**
     * All screens for this output. Screen IDs are the list indices.
     */
    protected final List<BLImageViewport> viewports = new ArrayList<BLImageViewport>();
    
    /**
     * The minimum amount of time between packets. All attempts to send a packet
     * less than this amount of time after the previously-sent packet will
     * result in no output.
     */
    protected final long minSendInterval;
    
    /**
     * The image to send a portion of. Viewport geometry is in terms of this
     * image. Normally this will be the virtual buffer, but under novel use of
     * the API, it could be any BLImage.
     */
    private final BLImage source;


    /**
     * @param source
     *            The image to send a portion of. Normally this will be the
     *            virtual buffer, but if you're making novel use of the API, it
     *            can be any BLImage.
     * @param packetType The type of packet to send
     * @param minSendInterval The minimum number of milliseconds to wait between
     * successive frames
     */
    public AbstractOutput(BLImage source, PacketType packetType, long minSendInterval) {
        this.source = source;
        this.packetType = packetType;
        this.minSendInterval = minSendInterval;
    }

    /* (non-Javadoc)
     * @see de.blinkenlights.bmix.mixer.Output#addScreen(java.awt.Rectangle, int)
     */
    public void addScreen(Rectangle bounds, int bpp) {
        if (viewports.size() > 0 && packetType != PacketType.MCU_MULTIFRAME) {
            throw new UnsupportedOperationException(
                    "Packet type " + packetType + " does not support multiple screens");
        }
        viewports.add(new BLImageViewport(source, bounds, bpp));
    }
    
    protected void sendSingleFrame(BLPacketSender sender, long lastSendTime, Logger logger)
    throws IOException {
        
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

    /* (non-Javadoc)
     * @see de.blinkenlights.bmix.mixer.Output#getViewports()
     */
    public List<BLImageViewport> getViewports() {
        return viewports;
    }

    /* (non-Javadoc)
     * @see de.blinkenlights.bmix.mixer.Output#getPacketType()
     */
    public PacketType getPacketType() {
        return packetType;
    }
 
    /* (non-Javadoc)
     * @see de.blinkenlights.bmix.mixer.Output#getMinSendInterval()
     */
    public long getMinSendInterval() {
        return minSendInterval;
    }

}
