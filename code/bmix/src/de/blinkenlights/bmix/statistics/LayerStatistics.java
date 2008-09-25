package de.blinkenlights.bmix.statistics;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Rectangle;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferInt;

import javax.swing.Icon;

import de.blinkenlights.bmix.mixer.Layer;

public class LayerStatistics implements StatisticsItem, Icon {

	private static final long serialVersionUID = 4810378087586013021L;
	
	/**
	 * The unique ID of the object these stats are about.
	 */
	private final long id;
	
	private final InputStatistics inputStat;
	private final Rectangle viewport;
	private final float opacity;
    private final Dimension imageSize;
    private final int[] imageData;
    private final boolean rootLayer;

    private transient BufferedImage frame;

    
	/**
	 * Creates a new LayerStatistics.
	 * 
	 * @param inputStat
	 *            the inputStatistics for this layer. If this layer has no
	 *            input, specify null.
	 * @param viewport
	 *            the viewport on the virtual framebuffer
	 * @param opacity
	 *            the opacity during mixdown
	 */
	public LayerStatistics(InputStatistics inputStat, Layer layer) {
		id = System.identityHashCode(layer);
		if (layer.getViewport() == null) {
			throw new NullPointerException("viewport was null");
		}
		this.inputStat = inputStat;
		viewport = layer.getViewport();
		opacity = layer.getOpacity();
		imageSize = new Dimension(layer.getImageWidth(), layer.getImageHeight());
		imageData = new int[imageSize.width * imageSize.height];
		layer.fillArray(imageData);
		rootLayer = (layer.getParentLayer() == null);
	}

	public long getId() {
		return id;
	}
	
	/**
	 * Returns the input statistics for this layer's input. If this layer
	 * has no input, the return value will be null.
	 */
	public InputStatistics getInputStat() {
		return inputStat;
	}

	public Rectangle getViewport() {
		return viewport;
	}

	public float getOpacity() {
		return opacity;
	}

	public String toString() {
		StringBuilder str = new StringBuilder();
		str.append("Layer - Source Input: " + (inputStat == null ? "none" : inputStat.getName()) + "\n");
		str.append("  Target viewport - x: " + viewport.x + " - y: " + viewport.y + 
				" - w: " + viewport.width + " - h: " + viewport.height + "\n");
		str.append("  Opacity: " + opacity + "\n");
		str.append("\n");
		return str.toString();
	}

	public String getName() {
		return "Layer";
	}

	public Dimension getImageSize() {
        return imageSize;
    }
	
	public int[] getImageData() {
        return imageData;
    }
	
	public String toHtml() {
		return String.format(
			"<html><table>" +
			"<tr><td>Source Input<td>%s" +
			"<tr><td>Target viewport<td>%dx%d+%d+%d" +
		    "<tr><td>Opacity<td>%.3f",
		    (inputStat == null ? "none" : inputStat.getName()),
		    viewport.width, viewport.height, viewport.x, viewport.y,
			opacity);
	}

    public int getIconHeight() {
        return getImageSize().height;
    }

    public int getIconWidth() {
        return getImageSize().width;
    }

    public void paintIcon(Component c, Graphics g, int x, int y) {
        if (frame == null) {
            frame = new BufferedImage(getIconWidth(), getIconHeight(), BufferedImage.TYPE_INT_ARGB);
            int[] pixels = ((DataBufferInt) frame.getRaster().getDataBuffer()).getData();
            if (pixels.length != imageData.length) {
                throw new AssertionError(
                        "Pixel sizes don't line up. Expected " +
                        pixels.length + ", got " + imageData.length);
            }
            System.arraycopy(imageData, 0, pixels, 0, pixels.length);
//            int nonzero = 0;
//            for (int i = 0; i < pixels.length; i++) {
//                pixels[i] = imageData[i];
//                if (pixels[i] != 0) {
//                    nonzero++;
//                }
//            }
//            System.err.println("Copied " + pixels.length + " pixels. nonzero=" + nonzero);
        }
        g.drawImage(frame, x, y, null);
    }

    public boolean isRootLayer() {
        return rootLayer;
    }
}
