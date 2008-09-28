package processing.blinkenlights;

import java.awt.image.BufferedImage;

/**
 * Instances of BLImage can provide a rectangle of pixel data. Each pixel has an
 * 8-bit alpha and an RGB value.
 */
public interface BLImage {
	
	/**
	 * Fills the passed BufferedImage with the content of this BLImage. If the given
	 * image is too small, the contents will be clipped, with the upper left corner as
	 * the anchor point. If the given image is too large, the extra pixels along the
	 * right and below will remain untouched.
	 * 
	 * @param target the buffered image to write into
	 */
	void fillBufferedImage(BufferedImage target);
	
	/**
	 * Returns the height in pixels of the content of this BLImage
	 */
	int getImageHeight();
	
	/**
	 * Returns the width in pixels of the content of this BLImage
	 */
	int getImageWidth();

}
