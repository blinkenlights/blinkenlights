package de.blinkenlights.bmix.statistics.gui;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Insets;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.image.BufferStrategy;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.swing.Icon;
import javax.swing.JPanel;
import javax.swing.SwingUtilities;

import de.blinkenlights.bmix.statistics.FrameStatistics;
import de.blinkenlights.bmix.statistics.InputStatistics;
import de.blinkenlights.bmix.statistics.LayerStatistics;
import de.blinkenlights.bmix.statistics.OutputStatistics;
import de.blinkenlights.bmix.statistics.StatisticsItem;

public class StatsComponent extends JPanel {

	private static final float[] DASH_PATTERN = new float[] {15, 7};

    private static class InfoBox {
		
		private InfoBox pointsTo;
		
		/**
		 * The component that paints the status in the GUI. Its bounds will
		 * be set to the preferred size, and will be relative to the top
		 * left corner of the overall stats component (not the column).
		 */
		private RoundedLabel display;
		
        private StatisticsItem stats;
		
		public InfoBox() {
			display = new RoundedLabel(4);
			display.setForeground(Color.WHITE);
		}

		public void updateStats(StatisticsItem stats) {
			this.stats = stats;
            display.setText(stats.toHtml());
            if (stats instanceof Icon) {
                display.setIcon((Icon) stats);
            } else {
                display.setIcon(null);
            }
			display.setSize(display.getPreferredSize());
		}

		/**
		 * Returns the appropriate dash phase for the line to pointsTo.
		 */
        public float getDashPhase() {
            if (pointsTo != null && pointsTo.stats instanceof InputStatistics) {
                long frameCount = ((InputStatistics) pointsTo.stats).getFrameCount();
                return frameCount;// % DASH_PATTERN.length;
                // TODO make additive and also figure out why this doesn't get called
            } else {
                return 0f;
            }
        }
	}
	
	/**
	 * Manages the positioning of infoboxes within a column.
	 */
	private class Column {
	    
	    Rectangle bounds = new Rectangle();
	    List<InfoBox> items = new ArrayList<InfoBox>();
	    
	    void addItem(InfoBox item) {
	        items.add(item);
	    }
	    
	    void removeItem(InfoBox item) {
	        items.remove(item);
	    }
	    
	    void paint(Graphics2D g) {
	        
	        fixLayout();
	        
	        for (InfoBox box : items) {
	            g.translate(box.display.getX(), box.display.getY());
	            box.display.paint(g);
                g.translate(-box.display.getX(), -box.display.getY());
	        }
	    }

        private void fixLayout() {
            int newHeight = 0;
            for (InfoBox box : items) {
                newHeight = Math.max(newHeight, box.display.getPreferredSize().height);
            }
            
            bounds.height = newHeight;
            
            int x = 0;
            for (InfoBox box : items) {
                Dimension infoBoxSize = box.display.getPreferredSize();
                box.display.setBounds(x, 0, infoBoxSize.width, bounds.height);
                x += box.display.getWidth();
            }
            
            bounds.width = x;
            bounds.x = StatsComponent.this.getWidth() / 2 - bounds.width / 2;
        }
	}
	private final Map<Long, InfoBox> infoBoxes = new HashMap<Long, InfoBox>();
	
    private final Column inputsColumn = new Column();
    private final Column layersColumn = new Column();
    private final Column rootLayerColumn = new Column();
    private final Column outputsColumn = new Column();
	
	public StatsComponent() {
	    setIgnoreRepaint(true);
		setBackground(Color.BLACK);
		setOpaque(true);
		setForeground(new Color(0xdddddd));
	}

	public void update(final FrameStatistics stats) {
	    for (InputStatistics is : stats.getInputStats()) {
	        InfoBox infoBox = infoBoxes.get(is.getId());
	        if (infoBox == null) {
	            infoBox = new InfoBox();
	            infoBoxes.put(is.getId(), infoBox);
	            inputsColumn.addItem(infoBox);
	        }
	        infoBox.updateStats(is);
	    }

	    for (LayerStatistics ls : stats.getLayerStats()) {
	        InfoBox infoBox = infoBoxes.get(ls.getId());
	        if (infoBox == null) {
	            infoBox = new InfoBox();
	            
	            if (ls.getInputStat() != null) {
	                InfoBox feeder = infoBoxes.get(ls.getInputStat().getId());
	                infoBox.pointsTo = feeder;
	            }
	            
	            infoBoxes.put(ls.getId(), infoBox);
	            
	            // XXX assumes flat hierarchy under root layer
	            if (ls.isRootLayer()) {
	                rootLayerColumn.addItem(infoBox);
	            } else {
	                layersColumn.addItem(infoBox);
	            }
	        }
	        infoBox.updateStats(ls);
	    }

	    for (OutputStatistics os : stats.getOutputStats()) {
	        InfoBox infoBox = infoBoxes.get(os.getId());
	        if (infoBox == null) {
	            infoBox = new InfoBox();
	            infoBoxes.put(os.getId(), infoBox);
	            outputsColumn.addItem(infoBox);
	        }
	        infoBox.updateStats(os);
	    }

	    // TODO reap unused infoboxes
	}
	
	@Override
	protected void paintComponent(Graphics g) {
	    // we don't paint this way
	}
	
	public void paint(BufferStrategy bufferStrategy, Insets insets) {
	    if (SwingUtilities.isEventDispatchThread()) {
	        throw new RuntimeException("We don't think swing will call this");
	    }
	    Graphics2D g = (Graphics2D) bufferStrategy.getDrawGraphics().create(
	            insets.left, insets.top, getWidth(), getHeight());
	    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
	    g.setColor(getBackground());
	    g.fillRect(0, 0, getWidth(), getHeight());
	    g.setColor(getForeground());
	    inputsColumn.bounds.y = 0;
        inputsColumn.paint(
                (Graphics2D) g.create(
                        inputsColumn.bounds.x, inputsColumn.bounds.y,
                        inputsColumn.bounds.width, inputsColumn.bounds.height));
        
        layersColumn.bounds.y = getHeight() / 2 - layersColumn.bounds.height;
        layersColumn.paint(
                (Graphics2D) g.create(
                        layersColumn.bounds.x, layersColumn.bounds.y,
                        layersColumn.bounds.width, layersColumn.bounds.height));
        
        for (InfoBox layerBox : layersColumn.items) {
            if (layerBox.pointsTo != null) {
                Point p1 = layerBox.display.getLocation();
                p1.y += layersColumn.bounds.y;
                p1.x += layersColumn.bounds.x + layerBox.display.getWidth() / 2;
                
                InfoBox sourceInputBox = layerBox.pointsTo;
                Point p2 = sourceInputBox.display.getLocation();
                p2.y += inputsColumn.bounds.y + sourceInputBox.display.getHeight();
                p2.x += inputsColumn.bounds.x + sourceInputBox.display.getWidth() / 2;
                
                g.setStroke(new BasicStroke(
                        1.5f, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND, 0f,
                        DASH_PATTERN, layerBox.getDashPhase()));
                g.drawLine(p1.x, p1.y, p2.x, p2.y);
            }
        }

        rootLayerColumn.bounds.y = getHeight() / 2;
        rootLayerColumn.paint(
                (Graphics2D) g.create(
                        rootLayerColumn.bounds.x, rootLayerColumn.bounds.y,
                        rootLayerColumn.bounds.width, rootLayerColumn.bounds.height));

        outputsColumn.bounds.y = getHeight() - outputsColumn.bounds.height;
        outputsColumn.paint(
                (Graphics2D) g.create(
                        outputsColumn.bounds.x, outputsColumn.bounds.y,
                        outputsColumn.bounds.width, outputsColumn.bounds.height));
        
        bufferStrategy.show();
	}
}
