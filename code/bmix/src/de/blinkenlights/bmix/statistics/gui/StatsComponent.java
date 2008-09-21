package de.blinkenlights.bmix.statistics.gui;

import java.awt.Color;
import java.util.HashMap;
import java.util.Map;

import javax.swing.JPanel;
import javax.swing.SwingUtilities;

import de.blinkenlights.bmix.statistics.FrameStatistics;
import de.blinkenlights.bmix.statistics.InputStatistics;
import de.blinkenlights.bmix.statistics.LayerStatistics;
import de.blinkenlights.bmix.statistics.OutputStatistics;
import de.blinkenlights.bmix.statistics.StatisticsItem;

public class StatsComponent extends JPanel {

	private static class InfoBox {
		
		private final long id;
		private InfoBox pointsTo;
		private RoundedLabel display;
		
		public InfoBox(long id) {
			this.id = id;
			this.display = new RoundedLabel();
			display.setForeground(Color.WHITE);
		}

		public void updateStats(StatisticsItem stats) {
			display.setText(stats.toHtml());
			display.setSize(display.getPreferredSize());
		}
	}
	
	private final Map<Long, InfoBox> infoBoxes = new HashMap<Long, InfoBox>();
	
	public StatsComponent() {
		setBackground(Color.BLACK);
		setOpaque(true);
		setForeground(new Color(0xdddddd));
	}
	
	public void update(final FrameStatistics stats) {
	    Runnable updateOnEDT = new Runnable() {
            public void run() {
                for (InputStatistics is : stats.getInputStats()) {
                    InfoBox infoBox = infoBoxes.get(is.getId());
                    if (infoBox == null) {
                        infoBox = new InfoBox(is.getId());
                        infoBoxes.put(is.getId(), infoBox);
                        add(infoBox.display);
                    }
                    infoBox.updateStats(is);
                }
                
                for (LayerStatistics ls : stats.getLayerStats()) {
                    InfoBox infoBox = infoBoxes.get(ls.getId());
                    if (infoBox == null) {
                        infoBox = new InfoBox(ls.getId());
                        infoBoxes.put(ls.getId(), infoBox);
                        add(infoBox.display);
                    }
                    infoBox.updateStats(ls);
                }
                
                for (OutputStatistics os : stats.getOutputStats()) {
                    InfoBox infoBox = infoBoxes.get(os.getId());
                    if (infoBox == null) {
                        infoBox = new InfoBox(os.getId());
                        infoBoxes.put(os.getId(), infoBox);
                        add(infoBox.display);
                    }
                    infoBox.updateStats(os);
                }
                
                // TODO reap unused infoboxes
                
                
            }
        };
        if (SwingUtilities.isEventDispatchThread()) {
            updateOnEDT.run();
        } else {
            SwingUtilities.invokeLater(updateOnEDT);
        }
	}
	
}
