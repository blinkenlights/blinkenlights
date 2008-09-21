package de.blinkenlights.bmix.statistics.gui;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Insets;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.image.BufferStrategy;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

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
		
		private final long id;
		private InfoBox pointsTo;
		
		/**
		 * The component that paints the status in the GUI. Its bounds will
		 * be set to the preferred size, and will be relative to the top
		 * left corner of the overall stats component (not the column).
		 */
		private RoundedLabel display;
        private StatisticsItem stats;
		
		public InfoBox(long id) {
			this.id = id;
			display = new RoundedLabel(4);
			display.setForeground(Color.WHITE);
		}

		public void updateStats(StatisticsItem stats) {
			this.stats = stats;
            display.setText(stats.toHtml());
			display.setSize(display.getPreferredSize());
		}

		/**
		 * Returns the appropriate dash phase for the line to pointsTo.
		 */
        public float getDashPhase() {
            if (pointsTo != null && pointsTo.stats instanceof InputStatistics) {
                long frameCount = ((InputStatistics) pointsTo.stats).getFrameCount();
                long phase = frameCount % 10;
                System.err.println("Phase: " + phase);
                return phase;
                // TODO make additive and also figure out why this doesn't get called
            } else {
                return 0f;
            }
        }
	}
	
	/**
	 * Manages the positioning of infoboxes within a column.
	 */
	private static class Column {
	    
	    Rectangle bounds = new Rectangle();
	    List<InfoBox> items = new ArrayList<InfoBox>();
	    
	    void addItem(InfoBox item) {
	        System.err.println("Item added on " + Thread.currentThread().getName());
	        items.add(item);
	    }
	    
	    void removeItem(InfoBox item) {
            System.err.println("Item removed on " + Thread.currentThread().getName());
	        items.remove(item);
	    }
	    
	    void paint(Graphics2D g) {
	        
	        fixLayout();
	        
	        for (InfoBox box : items) {
	            box.display.paint(g);
	            g.translate(0, box.display.getHeight());
	        }
	    }

        private void fixLayout() {
            int newWidth = 0;
            for (InfoBox box : items) {
                newWidth = Math.max(newWidth, box.display.getPreferredSize().width);
            }
            
            bounds.width = newWidth;
            
            int y = 0;
            for (InfoBox box : items) {
                Dimension infoBoxSize = box.display.getPreferredSize();
                box.display.setBounds(bounds.x, y, bounds.width, infoBoxSize.height);
                y += box.display.getHeight();
            }
            
            bounds.height = y;
        }
	}
	private final Map<Long, InfoBox> infoBoxes = new HashMap<Long, InfoBox>();
	
    private final Column inputsColumn = new Column();
    private final Column layersColumn = new Column();
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
	            infoBox = new InfoBox(is.getId());
	            infoBoxes.put(is.getId(), infoBox);
	            inputsColumn.addItem(infoBox);
	        }
	        infoBox.updateStats(is);
	    }

	    for (LayerStatistics ls : stats.getLayerStats()) {
	        InfoBox infoBox = infoBoxes.get(ls.getId());
	        if (infoBox == null) {
	            infoBox = new InfoBox(ls.getId());
	            
	            if (ls.getInputStat() != null) {
	                InfoBox feeder = infoBoxes.get(ls.getInputStat().getId());
	                infoBox.pointsTo = feeder;
	            }
	            
	            infoBoxes.put(ls.getId(), infoBox);
	            layersColumn.addItem(infoBox);
	        }
	        infoBox.updateStats(ls);
	    }

	    for (OutputStatistics os : stats.getOutputStats()) {
	        InfoBox infoBox = infoBoxes.get(os.getId());
	        if (infoBox == null) {
	            infoBox = new InfoBox(os.getId());
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
	    g.setColor(getBackground());
	    g.fillRect(0, 0, getWidth(), getHeight());
	    g.setColor(getForeground());
	    inputsColumn.bounds.x = 0;
        inputsColumn.paint(
                (Graphics2D) g.create(
                        inputsColumn.bounds.x, 0,
                        inputsColumn.bounds.width, inputsColumn.bounds.height));
        
        layersColumn.bounds.x = getWidth() / 2 - layersColumn.bounds.width / 2;
        layersColumn.paint(
                (Graphics2D) g.create(
                        layersColumn.bounds.x, 0,
                        layersColumn.bounds.width, layersColumn.bounds.height));
        
        for (InfoBox layerBox : layersColumn.items) {
            if (layerBox.pointsTo != null) {
                InfoBox sourceInputBox = layerBox.pointsTo;
                Point p1 = layerBox.display.getLocation();
                p1.y += layerBox.display.getHeight() / 2;
                
                Point p2 = sourceInputBox.display.getLocation();
                p2.y += sourceInputBox.display.getHeight() / 2;
                p2.x += sourceInputBox.display.getWidth();
                
                g.setStroke(new BasicStroke(
                        1.00001f, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND, 0f,
                        DASH_PATTERN, sourceInputBox.getDashPhase()));
                g.drawLine(p1.x, p1.y, p2.x, p2.y);
            }
        }
        
        outputsColumn.bounds.x = getWidth() - outputsColumn.bounds.width;
        outputsColumn.paint(
                (Graphics2D) g.create(
                        outputsColumn.bounds.x, 0,
                        outputsColumn.bounds.width, outputsColumn.bounds.height));
        
        bufferStrategy.show();
	}
}
