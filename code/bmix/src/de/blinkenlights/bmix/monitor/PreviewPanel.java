package de.blinkenlights.bmix.monitor;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.image.BufferedImage;

import javax.swing.JPanel;

/**
 * This class is the panel which draws BL frame data onto the screen.
 */
public class PreviewPanel extends JPanel {
	private static final long serialVersionUID = 1L;
	JPanel panel;
	BufferedImage img;
	long lastTime;
	boolean lines = false;
	NumberMuncher fps = new PeriodicAverage(new MovingAverage(1000), 1000);
	
	/**
	 * Creates a new PreviewFrame.
	 * 
	 */
	public PreviewPanel() {
		lastTime = System.currentTimeMillis();
		setOpaque(true);
	}
	
	
	/**
	 * Sets the blinkScreen.
	 * 
	 * @param blinkScreen the image to display
	 */
	public void setBlinkScreen(BufferedImage blinkScreen) {
		this.img = blinkScreen;
		long currentTime = System.currentTimeMillis();
		fps.putValue(currentTime - lastTime);
		lastTime = currentTime;
		this.repaint();
	}
	
	
	/**
	 * Paints the component.
	 * 
	 * @param g the graphics context to use
	 */
	public void paintComponent(Graphics g) {
		Graphics2D g2 = (Graphics2D)g;
		
        g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_OFF);
        g2.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR);
		g2.setColor(Color.BLACK);
		g2.fillRect(0, 0, this.getWidth(), this.getHeight());	
		
		if(img == null) return;
		
		// draw the image
		g2.drawImage(img, 0, 0, this.getWidth(), this.getHeight(), null);
		
		// draw lines
		if (lines) {
            g2.setColor(Color.GRAY);
            double stepX = (double) this.getWidth() / (double) img.getWidth();
            double stepY = (double) this.getHeight() / (double) img.getHeight();

            for (int i = 1; i < img.getWidth(); i++) {
                g2.drawLine((int) ((double) i * stepX), 0,
                        (int) ((double) i * stepX), this.getHeight() - 1);
            }

            for (int i = 1; i < img.getHeight(); i++) {
                g2.drawLine(0, (int) ((double) i * stepY), this.getWidth() - 1,
                        (int) ((double) i * stepY));
            }
        }
		
		// draw stats
		g2.setColor(Color.WHITE);
		g2.drawString("Source Size: " + img.getWidth() + " x " + img.getHeight(), 20, 20);
		g2.drawString(String.format("Framerate: %3.1f fps", 1000f / fps.getCurrentValue()), 20, 20 + g2.getFontMetrics().getHeight());
	}
}
