package de.blinkenlights.bmix.mixer;

import java.awt.Rectangle;

import de.blinkenlights.bmix.network.BLNetworkException;
import de.blinkenlights.bmix.network.BLPacketSender;
import de.blinkenlights.bmix.protocol.BLFramePacket;

/**
 * This class is an output which gets pixels from a source image and sends
 * BLFramePackets to a destination.
 */
public class Output {
    private final BLPacketSender sender;
    private final BLImageViewport viewport;

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
    public Output(BLPacketSender sender, BLImage source, Rectangle viewport) {
        super();
        this.sender = sender;
        this.viewport = new BLImageViewport(source, viewport);
    }

    /**
     * Reads the data in this output's viewport rectangle from the given BLImage
     * and sends it via this output's BLPacketSender.
     * 
     * @throws BLNetworkException if there was an error sending the packet
     */
    public void send() throws BLNetworkException {
        BLFramePacket p = new BLFramePacket(viewport, 1, 255);
        sender.send(p.getNetworkBytes());
    }
    
    @Override
    public String toString() {
        return "" + sender + " " + viewport;
    }
}
