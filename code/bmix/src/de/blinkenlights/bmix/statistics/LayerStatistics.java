package de.blinkenlights.bmix.statistics;

import java.awt.Rectangle;

public class LayerStatistics {
	private final InputStatistics inputStat;
	private final Rectangle viewport;
	private final float opacity;

	/**
	 * Creates a new LayerStatistics.
	 * 
	 * @param inputStat the inputStatistics for this layer
	 * @param viewport the viewport on the virtual framebuffer
	 * @param opacity the opacity during mixdown
	 */
	public LayerStatistics(InputStatistics inputStat, Rectangle viewport, float opacity) {
		this.inputStat = inputStat;
		this.viewport = viewport;
		this.opacity = opacity;
	}

	public InputStatistics getInputStat() {
		return inputStat;
	}

	public Rectangle getViewport() {
		return viewport;
	}

	public float getOpacity() {
		return opacity;
	}

}
