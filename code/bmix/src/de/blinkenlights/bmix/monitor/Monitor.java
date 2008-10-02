package de.blinkenlights.bmix.monitor;

import java.awt.Dimension;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.image.BufferedImage;
import java.util.prefs.BackingStoreException;

import javax.swing.JFrame;
import javax.swing.SwingUtilities;

import de.blinkenlights.bmix.mixer.BLImage;

/**
 * This class implements a Swing GUI to display some BL frame stream data on the screen. 
 */
public abstract class Monitor extends Thread {
	private static final long serialVersionUID = 1L;
	private JFrame frame;
	private PreviewPanel previewPanel;
	
	String name;
	int x;
	int y;
	int w;
	int h;
	

	/**
	 * Creates a new PreviewGUI.
	 */
	public Monitor(String name, int x, int y, int w, int h, boolean guiEnabled) {
		this.name = name;
		this.x = x;
		this.y = y;
		this.w = w;
		this.h = h;
		if (guiEnabled) {
			setupGUI();
		}
	}
	

	/**
	 * Sets up the GUI.
	 */
	private void setupGUI() {
		frame = new JFrame(name);
		
		frame.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
	    
	    frame.addWindowListener(new WindowAdapter() {
         public void windowClosing(WindowEvent e) {
             shutdown();
             System.exit(0);
         }
	    });
		
		previewPanel = new PreviewPanel();
		previewPanel.setPreferredSize(new Dimension(w, h));
		frame.add(previewPanel);
		SwingUtilities.invokeLater(new Runnable() {
			public void run() {
				frame.pack();
				frame.setLocation(x, y);
				frame.setVisible(true);
			}
		});
		
	}
	
	 
	public void shutdown() {
		
	}
	
	/**
	 * Run the thread.
	 */
	public void run() {
		if (SwingUtilities.isEventDispatchThread()) {
			throw new IllegalStateException("can't run this from the event dispatch thread!");
		}
		BufferedImage bi = null;				
		while (true) {
			BLImage blim = getNextImage();
			if (bi == null || bi.getWidth() != blim.getImageWidth() || bi.getHeight() != blim.getImageHeight()) {
				bi = new BufferedImage(blim.getImageWidth(),blim.getImageHeight(), BufferedImage.TYPE_INT_ARGB);				
			}
			blim.fillBufferedImage(bi);
			if (previewPanel != null) {
				previewPanel.setBlinkScreen(bi);
			}
		}

	}
	
	
	/**
	 * Returns the next image that should be displayed on this preview buffer.
	 * This method can block for as long as it wants (for instance, if the image
	 * comes from a network socket or user input, or 1541 floppy disk).
	 */
	protected abstract BLImage getNextImage();
}
