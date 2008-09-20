package de.blinkenlights.bmix.statistics;

import java.awt.Rectangle;
import java.util.Collections;
import java.util.List;

public class LayerStatistics implements StatisticsItem {

	private static final long serialVersionUID = 4810378087586013021L;
	
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

	public List<StatisticsItem> getChildren() {
		return Collections.emptyList();
	}

	public String getName() {
		return "Layer";
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
}
