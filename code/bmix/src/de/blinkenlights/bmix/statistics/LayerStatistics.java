package de.blinkenlights.bmix.statistics;

import java.awt.Rectangle;

public class LayerStatistics {
	private final InputStatistics inputStat;
	private final Rectangle viewport;
	private final float opacity;

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
	public LayerStatistics(InputStatistics inputStat, Rectangle viewport, float opacity) {
		if (viewport == null) {
			throw new NullPointerException("viewport was null");
		}
		this.inputStat = inputStat;
		this.viewport = viewport;
		this.opacity = opacity;
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
}
