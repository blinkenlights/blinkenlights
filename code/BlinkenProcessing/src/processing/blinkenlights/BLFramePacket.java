package processing.blinkenlights;

import java.awt.Color;

/**
 * This class is a BLFramePacket. It translates image data into network bytes to
 * send over a network. It can also translate network bytes into an image.
 */
public class BLFramePacket extends AbstractFramePacket {
	
	/**
	 * Creates a new BLFramePacket.
	 * 
	 * @param width
	 *            the width in pixels
	 * @param height
	 *            the height in pixels
	 * @param channels
	 *            the number of colour channels
	 * @param maxval
	 *            the max pixel value
	 * @param pixelData
	 *            the pixel data. Must not be null, and must have length of
	 *            {@link #width} {@link #height} {@link #channels}.
	 * @param alphaMode
	 *            The method for converting pixels from the network format to
	 *            our standard 32-bit ARGB format.
	 * @param transparentColour
	 *            The colour in the input that should be treated as transparent.
	 *            If not using {@link AlphaMode#CHROMA_KEY}, pass in null for
	 *            this parameter, because it is ignored.
	 */
	public BLFramePacket(int width, int height,
			byte pixelData[], AlphaMode alphaMode, Color transparentColour) {
		super(width, height, pixelData, alphaMode, transparentColour);
	}
	
	/**
	 * Creates a new BLFramePacket with the pixel data initialized from the given image.
	 * 
	 * @param image the image to use for the data
	 */
	public BLFramePacket(BLImage image) {
		super(image);
	}
	
	/**
	 * Gets contents of this object in MCU_FRAME_PACKET network protocol format.
	 * The output will have one channel and a maxval of 255.
	 * 
	 * @return the network protocol bytes
	 */
	public byte[] getNetworkBytes() {
		final int width = getImageWidth();
		final int height = getImageHeight();
		final int maxval = 0xff;
		final int channels = 1;
		byte buf[] = new byte[12 + (width * height)];
		buf[0] = 0x23;
		buf[1] = 0x54;
		buf[2] = 0x26;
		buf[3] = 0x66;
		buf[4] = (byte)((height & 0xff00) >> 8);
		buf[5] = (byte)(height & 0x00ff); 
		buf[6] = (byte)((width & 0xff00) >> 8);
		buf[7] = (byte)(width & 0x00ff); 
		buf[8] = (byte)((channels & 0xff00) >> 8);
		buf[9] = (byte)(channels & 0x00ff); 
		buf[10] = (byte)((maxval & 0xff00) >> 8);
		buf[11] = (byte)(maxval & 0x00ff); 
		System.arraycopy(pixelData, 0, buf, 12, pixelData.length);
		return buf;
	}
}
