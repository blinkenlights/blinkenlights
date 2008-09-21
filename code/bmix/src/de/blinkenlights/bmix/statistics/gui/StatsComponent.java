package de.blinkenlights.bmix.statistics.gui;

import java.awt.Color;
import java.awt.Graphics;
import java.util.HashMap;
import java.util.Map;

import javax.swing.JLabel;
import javax.swing.JPanel;

import de.blinkenlights.bmix.statistics.FrameStatistics;
import de.blinkenlights.bmix.statistics.InputStatistics;
import de.blinkenlights.bmix.statistics.StatisticsItem;

public class StatsComponent extends JPanel {

	private static class InfoBox {
		
		private final long id;
		private InfoBox pointsTo;
		private JLabel display;
		
		public InfoBox(long id) {
			this.id = id;
			this.display = new JLabel();
		}

		public void updateStats(StatisticsItem stats) {
			display.setText(stats.toHtml());
			display.setSize(display.getPreferredSize());
		}
	}
	
	private final Map<Long, InfoBox> infoBoxes = new HashMap<Long, InfoBox>();
	
	public StatsComponent() {
		setBackground(Color.BLACK);
		setForeground(new Color(0xdddddd));
	}
	
	public void update(FrameStatistics stats) {
		for (InputStatistics is : stats.getInputStats()) {
			InfoBox infoBox = infoBoxes.get(is.getId());
			if (infoBox == null) {
				infoBox = new InfoBox(is.getId());
			}
		}
	}
	
	@Override
	protected void paintComponent(Graphics g) {
		super.paintComponent(g);
		g = g.create();
		
		// TODO 3 columns
		for (InfoBox box : infoBoxes.values()) {
			box.display.paint(g);
			g.translate(box.display.getWidth(), 0);
		}
	}
}
