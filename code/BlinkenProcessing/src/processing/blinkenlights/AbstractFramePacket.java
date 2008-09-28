package processing.blinkenlights;

import java.awt.Color;
import java.awt.image.BufferedImage;

/**
 * This class is the common parts of all types of BLFramePacket. It must be subclassed to provide the network byte generation code.
 */
public abstract class AbstractFramePacket implements BLPacket, BLImage {
	
	
	public static enum AlphaMode {
		/** alpha channel ignored even if present */
		OPAQUE("opaque"),
		
		/** one colour is fully transparent; all others are opaque */
		CHROMA_KEY("chroma-key"),
		
		/** all pixels are full white, and opacity is set to brightness */
		BRIGHTNESS("brightness"),
		
		/** magically delicious undocumented mode */
		NATIVE("native");
		
		private String code;

		AlphaMode(String code) {
			this.code = code;
		}
		
		public static AlphaMode forCode(String code) {
			for (AlphaMode am : values()) {
				if (am.code.equals(code)) {
					return am;
				}
			}
			throw new IllegalArgumentException("Unknown code: " + code);
		}
		
		@Override
		public String toString() {
			return code;
		}
	}
	
	/**
	 * Number of pixels across the frame.
	 */
	private final int width;
	
	/**
	 * Number of pixels from top to bottom in the frame.
	 */
	private final int height;

	/**
	 * Rows of pixel data, starting at top left corner. Each position is a
	 * pixel, with value 0..255 inclusive. The length of this array will be the
	 * same as {@link #width} * {@link #height}.
	 */
	protected final byte pixelData[];

	private final Color transparentColour;

	private final AlphaMode alphaMode;

	/**
	 * Creates a new BLFramePacket.
	 * 
	 * @param width
	 *            the width in pixels
	 * @param height
	 *            the height in pixels
	 * @param pixelData
	 *            the pixel data. Must not be null, and must have length of
	 *            {@link #width} * {@link #height}. Pixel values must be normalized
	 *            to the range 0..255.
	 * @param alphaMode
	 *            The method for converting pixels from the network format to
	 *            our standard 32-bit ARGB format.
	 * @param transparentColour
	 *            The colour in the input that should be treated as transparent.
	 *            If not using {@link AlphaMode#CHROMA_KEY}, pass in null for
	 *            this parameter, because it is ignored.
	 */
	public AbstractFramePacket(int width, int height,
			byte pixelData[], AlphaMode alphaMode, Color transparentColour) {
		this.width = width;
		this.height = height;
		this.alphaMode = alphaMode;
		if (alphaMode == AlphaMode.CHROMA_KEY && transparentColour == null) {
			throw new IllegalArgumentException("Alpha mode is CHROMA_KEY but transparentColour was null");
		}
		this.transparentColour = transparentColour;
		this.pixelData = pixelData;
		
		if (pixelData.length != width * height) {
			throw new IllegalArgumentException(
					"Array length not appropriate to frame dimensions." +
					" pixelData.length=" + pixelData.length +
					"; width = " + width +
					"; height = " + height +
					" (expected length = " + (width * height) + ")");
		}		
	}
	
	
	/**
	 * Creates a new BLFramePacket. Uses the OPAQUE alpha mode.
	 * 
	 * @param image the image to use for the data
	 * @param channels the number of channels
	 * @param maxval the max pixel value
	 */
	public AbstractFramePacket(BLImage image) {
		width = image.getImageWidth();
		height = image.getImageHeight();
		transparentColour = null;
		alphaMode = AlphaMode.OPAQUE;
		pixelData = new byte[width * height];
		BufferedImage img = new BufferedImage(
				image.getImageWidth(), image.getImageHeight(),
				BufferedImage.TYPE_INT_ARGB);
		image.fillBufferedImage(img);

		int pixelCount = 0;
		for (int k = 0; k < img.getHeight(); k++) {
			for (int j = 0; j < img.getWidth(); j++) {
				pixelData[pixelCount] = (byte) (img.getRGB(j, k) & 0x000000ff);
				pixelCount++;
			}
		}
	}

	public int getImageWidth() {
		return width;
	}

	public int getImageHeight() {
		return height;
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
		int offs = (x + (y * width));
		int red;
		int green;
		int blue;
		red = pixelData[offs] & 0xff;
		green = pixelData[offs] & 0xff;
		blue = pixelData[offs] & 0xff;
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
     *            the BufferedImage to fill
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
	public abstract byte[] getNetworkBytes();
	
	/**
	 * Creates a String representing the frame.
	 * 
	 * @return a String representing the frame
	 */
	public String toString() {
		String str = "FRAME - w: " + width + " - h: " + height + "\n";
		for(int j = 0; j < height; j ++) {
			for(int i = 0; i < width; i ++) {
				str += Integer.toHexString((pixelData[(j * width) + i]) & 0xf);
			}
			str += "\n";
		}
		return str;
	}
}
