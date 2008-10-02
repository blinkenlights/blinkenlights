package de.blinkenlights.bmix.mixer;

import java.awt.AlphaComposite;
import java.awt.Color;
import java.awt.Composite;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferInt;
import java.util.ArrayList;
import java.util.List;

/**
 * This class is a Layer in the mixing process.
 */
public class Layer implements BLImage {
	
	/**
	 * This is our place on the {@link #parentLayer} where we are to draw our image.
	 */
	private final Rectangle viewport;
	
	/**
	 * The storage for our current image frame to be placed on the framebuffer.
	 */
	private final BufferedImage bi;
	
	/**
	 * Access to the pixel data in bi.
	 */
	private final int[] biPixels;
	
	/** 
	 * Compositing rule used when drawing this layer onto its parent layer.
	 */
	private final Composite composite;

    /**
     * The layer this layer belongs to. Layers that are unparented (including
     * the root layer) will have a null parent layer.
     */
	private Layer parentLayer;
	
	/**
	 * The layers of this virtual frame buffer. The bottommost layer
	 * is first in the list.
	 */
	private final List<Layer> layers = new ArrayList<Layer>();
	
	public Layer(Rectangle viewport, Composite composite) {
		this.composite = composite;
		this.viewport = new Rectangle(viewport);
		bi = new BufferedImage(viewport.width,viewport.height,BufferedImage.TYPE_INT_ARGB);
		biPixels = ((DataBufferInt) bi.getRaster().getDataBuffer()).getData();
	}

    /**
     * Returns the layer this layer belongs to. Layers that are unparented
     * (including the root layer) will have a null parent layer.
     */
	public Layer getParentLayer() {
        return parentLayer;
    }
	
	/**
	 * Copies the image data from this layer's input into this layer's buffered image {@link #bi}.
	 * If the input's current frame is not the same size as this layer, this operation will scale
	 * or crop the input's image according to the input's settings.
	 * 
	 * @param newImage The image to copy to this layer.
	 */
	public void updateImage(BufferedImage newImage, Point offset) {
		
	    clearImageToTransparent();

	    Graphics2D g = (Graphics2D) bi.getGraphics();
	    if (offset != null) {
	    	// no scaling, but an offset into the source image
	    	g.drawImage(newImage,
	    			0, 0, bi.getWidth(), bi.getHeight(),
	    			offset.x, offset.y, offset.x + bi.getWidth(), offset.y + bi.getHeight(), null);
	    } else {
	    	// scale source image to fit bi
	    	g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_OFF);
	    	g.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR);
	    	g.drawImage(newImage, 0, 0, bi.getWidth(), bi.getHeight(), 0, 0, newImage.getWidth(), newImage.getHeight(), null);
	    }
		g.dispose();
	}
	
	/**
	 * Resets this layer's own image contents to opaque black. Normally you will
	 * not need to call this, since {@link #updateImage(BufferedImage)} resets
	 * the layer to fully transparent before copying in its source image.
	 * The original use for this method was to clear the root (bottommost) layer
	 * when compositing.
	 */
	public void clearImageToBlack() {
		Graphics g = bi.getGraphics();
		g.setColor(Color.BLACK);
		g.fillRect(0, 0, bi.getWidth(), bi.getHeight());
		g.dispose();
	}
	
	public void clearImageToTransparent() {
		for (int i = 0; i < biPixels.length; i++) {
			biPixels[i] = 0;
		}
	}
	
	public void fillBufferedImage(BufferedImage bi) {
		this.bi.copyData(bi.getRaster());
	}

	public int getImageHeight() {
		return this.bi.getHeight();
	}

	public int getImageWidth() {
		return bi.getWidth();
	}
	
	/**
	 * Adds the given layer on bottom of the layer stack.
	 */
	public void addLayer(Layer layer) {
		if (layer == null) {
			throw new NullPointerException("Null layer not allowed in layer stack");
		}
		layers.add(0, layer);
		layer.parentLayer = this;
	}

	
	public List<Layer> getLayers() {
		return layers;
	}
	
	
    /**
     * Performs a mixdown of all the layers descended from this one. After
     * calling this method, pick up your mixed down pixels via
     * {@link #fillBufferedImage(BufferedImage)}.
     */
	public void mixdown() {
	    clearImageToBlack();
	    Graphics2D g2 = (Graphics2D) bi.getGraphics();
	    mixdown(g2);
	    g2.dispose();
	}
	
	/**
	 * Private recursive subroutine of {@link #mixdown()}.
	 */
	private void mixdown(Graphics2D g2) {
		draw(g2);
		for (Layer l : layers) {
			Rectangle r = l.viewport;
			Graphics2D childg2 = (Graphics2D) g2.create(r.x, r.y, r.width, r.height);
			l.mixdown(childg2);
			childg2.dispose();
		}
	}

	/**
	 * Draws this layer's current input image into the given graphics.
	 * 
	 * @param g The graphics to draw into (often this is the parent layer's graphics)
	 */
	public void draw(Graphics2D g) {
		Composite backupComposite = g.getComposite();
		g.setComposite(composite);
		g.drawImage(bi, 0, 0, null);
		g.setComposite(backupComposite);
	}

	public Rectangle getViewport() {
		return new Rectangle(viewport);
	}
	
	public float getOpacity() {
		if(composite instanceof AlphaComposite) {
			AlphaComposite ac = (AlphaComposite) composite;
			return ac.getAlpha();
		}
		return 1.0f;
	}

    public void fillArray(int[] imageData) {
        if (imageData.length != biPixels.length) {
            throw new IllegalArgumentException(
                    "Incorrect array size. I need: " +
                    biPixels.length + "; You gave: " + imageData.length);
        }
        System.arraycopy(biPixels, 0, imageData, 0, biPixels.length);
    }
}
