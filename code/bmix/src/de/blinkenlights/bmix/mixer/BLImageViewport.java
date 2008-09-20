package de.blinkenlights.bmix.mixer;

import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.image.BufferedImage;

/**
 * This class is a viewport to an image source used for creating outputs
 * from the main mixdown buffer.
 */
public class BLImageViewport implements BLImage {
    private final BLImage source;
    private final BufferedImage sourceImage;
    private final Rectangle viewport;

    /**
     * Creates a new BLImageViewport
     * 
     * @param source the source image to use
     * @param viewport the Rectangle specifying the rectangle for this viewport
     */
    public BLImageViewport(BLImage source, Rectangle viewport) {
        this.source = source;
        this.sourceImage = new BufferedImage(source.getImageWidth(), source.getImageHeight(), BufferedImage.TYPE_INT_ARGB);
        this.viewport = viewport;
    }

    /**
     * Fills bi with the cropped version of the image this viewport wraps.
     */
    public void fillBufferedImage(BufferedImage bi) {
        source.fillBufferedImage(sourceImage);
        Graphics2D big = bi.createGraphics();
        big.drawImage(
                sourceImage,
                0, 0, viewport.width, viewport.height,
                viewport.x, viewport.y, viewport.x + viewport.width, viewport.y + viewport.height,
                null);
        big.dispose();
    }

    /**
     * Returns the viewport's height.
     */
    public int getImageHeight() {
        return viewport.height;
    }

    /**
     * Returns the viewport's width.
     */
    public int getImageWidth() {
        return viewport.width;
    }
    
    public Rectangle getViewport() {
		return viewport;
	}

	@Override
    public String toString() {
        return "viewport " + viewport;
    }
}
