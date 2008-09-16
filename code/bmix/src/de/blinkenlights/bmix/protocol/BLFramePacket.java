package de.blinkenlights.bmix.protocol;

import java.awt.Color;
import java.awt.image.BufferedImage;

import de.blinkenlights.bmix.mixer.BLImage;
import de.blinkenlights.bmix.network.BLPacketReceiver.AlphaMode;

/**
 * This class is a BLFramePacket. It translates image data into network bytes to
 * send over a network. It can also translate network bytes into an image.
 */
public class BLFramePacket implements BLPacket, BLImage {
	
	/**
	 * Number of pixels across the frame.
	 */
	private final int width;
	
	/**
	 * Number of pixels from top to bottom in the frame.
	 */
	private final int height;
	
	/**
	 * The number of colour channels in this frame (currently defined
	 * as 1 or 3 in the protocol).
	 */
	private final int channels;
	
	/**
	 * Maximum value of a pixel. This pixel value means "full on."
	 */
	private final int maxval;

	/**
	 * Rows of pixel data, starting at top left corner. Each position is a
	 * channel of a pixel, and will have a value between 0..{@link #maxval}
	 * inclusive. The channel values are interleaved, so the length of this
	 * array will be the same as {@link #width} {@link #height} *
	 * {@link #channels}.
	 */
	private final byte pixelData[];

	private final Color transparentColour;

	private final AlphaMode alphaMode;

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
	public BLFramePacket(int width, int height, int channels,
			byte pixelData[], AlphaMode alphaMode, Color transparentColour) {
		this.width = width;
		this.height = height;
		this.channels = channels;
		this.alphaMode = alphaMode;
		if (alphaMode == AlphaMode.CHROMA_KEY && transparentColour == null) {
			throw new IllegalArgumentException("Alpha mode is CHROMA_KEY but transparentColour was null");
		}
		this.transparentColour = transparentColour;
		this.maxval = 255; // received data maxval is always 255
		this.pixelData = pixelData;
		
		if (channels != 1 && channels != 3) {
			throw new IllegalArgumentException("Non-allowed channels value: " + channels);
		}
		if (pixelData.length != width * height * channels) {
			throw new IllegalArgumentException(
					"Array length not appropriate to frame dimensions." +
					" pixelData.length=" + pixelData.length +
					"; width = " + width +
					"; height = " + height +
					"; channels = " + channels +
					" (expected length = " + (width * height * channels) + ")");
		}		
	}
	
	
	/**
	 * Creates a new BLFramePacket.
	 * 
	 * @param image the image to use for the data
	 * @param channels the number of channels
	 * @param maxval the max pixel value
	 */
	public BLFramePacket(BLImage image, int channels, int maxval) {
		width = image.getImageWidth();
		height = image.getImageHeight();
		this.channels = channels;
		this.maxval = maxval;
		transparentColour = null;
		alphaMode = AlphaMode.OPAQUE;
		if(channels != 1) {
			throw new IllegalArgumentException("channels > 1 are not supported, request channels: "+channels);
		}
		pixelData = new byte[width * height];
		BufferedImage img = new BufferedImage(image.getImageWidth(), image.getImageHeight(), BufferedImage.TYPE_INT_ARGB);
		image.fillBufferedImage(img);
		
		int pixelCount = 0;
		for(int k = 0; k < img.getHeight(); k ++) {
			for(int j = 0; j < img.getWidth(); j ++) {
				pixelData[pixelCount] = (byte)(img.getRGB(j, k) & 0x000000ff);
				pixelCount ++;
			}
		}
	}
	

	public int getImageWidth() {
		return width;
	}


	public int getImageHeight() {
		return height;
	}


	public int getChannels() {
		return channels;
	}


	public int getMaxval() {
		return maxval;
	}

	
	/**
	 * Returns a ninteger set up like this: 0xAARRGGBB (i.e.&nbsp;middle endian).
	 * 
	 * @param x
	 *            The x coordinate (0..width-1).
	 * @param y
	 *            The y coordinate (0..height-1).
	 * @return an integer packed as follows: 0xAARRGGBB
	 */
	public int getARGB(int x, int y, AlphaMode alphaMode) {
		if (x >= width || x < 0) {
			throw new IndexOutOfBoundsException("x = " + x + "; width = " + width);
		}
		if (y >= height || y < 0) {
			throw new IndexOutOfBoundsException("y = " + y + "; height = " + height);
		}
		int offs = (x + (y * width)) * channels;
		int red;
		int green;
		int blue;
		if (channels == 1) {
			red = pixelData[offs] & 0xff;
			green = pixelData[offs] & 0xff;
			blue = pixelData[offs] & 0xff;
		} else if (channels == 3) {
			red = pixelData[offs + 0] & 0xff;
			green = pixelData[offs + 1] & 0xff;
			blue = pixelData[offs + 2] & 0xff;
		} else {
			throw new AssertionError("Computer needs new RAM");
		}
		int argb = (red << 16) | (green << 8) | blue;
		int alpha;
		if (alphaMode == AlphaMode.NATIVE) {
			if (0 == argb) {
				alpha = 0x00;
			} else {
				alpha = 0xff;
			}
		} else if (alphaMode == AlphaMode.CHROMA_KEY) {
			if (transparentColour != null &&
					(transparentColour.getRGB() & 0xffffff) == argb) {
				alpha = 0x00;
			} else {
				alpha = 0xff;
			}
		} else if (alphaMode == AlphaMode.OPAQUE) {
			alpha = 0xff;
		} else if (alphaMode == AlphaMode.BRIGHTNESS) {
			alpha = red;
			argb = 0xffffff;
		} else {
			throw new IllegalArgumentException("Unknown/unsupported alpha mode: " + alphaMode);
		}
		argb |= (alpha << 24);
		return argb;
	}

    /**
     * Fills a BufferedImage with the current frame packet. Guaranteed to set
     * every pixel in the given image.
     * 
     * @param target
     *            the BufferedImage to fil
     */
	public void fillBufferedImage(BufferedImage target) {
		int width = Math.min(this.width, target.getWidth());
		int height = Math.min(this.height, target.getHeight());		
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				int argb = getARGB(i, j, alphaMode);
				target.setRGB(i, j, argb);
			}
		}
	}
	
	/**
	 * Gets contents of this object in network protocol format.
	 * 
	 * @return the network protocol bytes
	 */
	public byte[] getNetworkBytes() {
		byte buf[] = new byte[12 + (width * height * channels)];
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
	
	
	/**
	 * Creates a String representing the frame.
	 * 
	 * @return a String representing the frame
	 */
	public String toString() {
		String str = "FRAME - w: " + width + " - h: " + height + " - channels: " + channels + 
			" - maxval: " + maxval + "\n";
		for(int j = 0; j < height; j ++) {
			for(int i = 0; i < width; i ++) {
				str += Integer.toHexString((pixelData[(j * width) + i]) & 0xf);
			}
			str += "\n";
		}
		return str;
	}
}
