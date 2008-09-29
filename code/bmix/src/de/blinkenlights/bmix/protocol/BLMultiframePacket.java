package de.blinkenlights.bmix.protocol;

import java.io.ByteArrayOutputStream;
import java.util.List;

import de.blinkenlights.bmix.mixer.BLImageViewport;

/**
 * The BLMultiframePacket represents a network packet of one or
 * more subframes (each having independent side and bpp) which
 * will be transformed into a single MCU_MULTIFRAME network packet.
 */
public class BLMultiframePacket {

	private final List<BLImageViewport> viewports;

	private final long tstamp = System.currentTimeMillis();
	
    /**
	 * Creates a new Frame Packet. The timestamp for the packet is the time
	 * of object creation so that multiple copies bear the same timestamp.
	 * However, this means that instances of this class go out of date quickly,
	 * so be sure to create the packet just before sending it.
	 * 
	 * @param image
	 *            the image to use for the data
	 */
	public BLMultiframePacket(List<BLImageViewport> viewports) {
        this.viewports = viewports;
	}

	public byte[] getNetworkBytes() {
	    
		ByteArrayOutputStream buf = new ByteArrayOutputStream();
		buf.write((byte) 0x23);
		buf.write((byte) 0x54);
		buf.write((byte) 0x26);
		buf.write((byte) 0x68);
		for (int i = 7; i >= 0; i--) {
			buf.write((byte) ((tstamp >> 8*i) & 0xff));
		}
		for (int screen = 0; screen < viewports.size(); screen++) {
		    
		    BLImageViewport viewport = viewports.get(screen);
		    byte[] pixelData = AbstractFramePacket.extractPixelData(viewport);
		    
		    buf.write((byte) screen);
		    buf.write((byte) (viewport.getBpp() & 0xff));
		    
		    // height (16-bit)
		    buf.write((byte) ((viewport.getImageHeight() >> 8) & 0xff));
		    buf.write((byte) (viewport.getImageHeight() & 0xff));

		    // width (16-bit)
		    buf.write((byte) ((viewport.getImageWidth() >> 8) & 0xff));
		    buf.write((byte) (viewport.getImageWidth() & 0xff));

		    for (int j = 0; j < viewport.getImageHeight(); j++) {
		        if (viewport.getBpp() == 4) {
		            for (int i = 0; i < viewport.getImageWidth(); i += 2) {
		                int left = pixelData[i + (j * viewport.getImageWidth())] >> 4;
		            int right;
		            if(i + 1 < viewport.getImageWidth()) {
		                right = pixelData[i + (j * viewport.getImageWidth()) + 1] >> 4;
		            } else {
		                right = 0;
		            }
		            buf.write((byte)((left << 4 | (right & 0x0f)) & 0xff));
		            }
		        }
		        if (viewport.getBpp() == 8) {
		            for (int i = 0; i < viewport.getImageWidth(); i++) {
		                buf.write(pixelData[i + (j * viewport.getImageWidth())]);
		            }
		        }
		    }
		}
		return buf.toByteArray();
	}
}
