package de.blinkenlights.bmix.movie;

import java.awt.Graphics2D;
import java.awt.image.BufferedImage;

import de.blinkenlights.bmix.mixer.BLImage;

/**
 * This class represents a frame from a BLM movie. It knows how to parse row data from a BLM
 * movie file and can generate a BLImage object from its contents.
 * 
 * Currently only 1 and 4 bit images are supported. In the case of a 1 bit image, valid pixel
 * values are '1' for on, and '0' for off. Other characters will be interpreted as off.
 * 
 * In the case of 4 bit images, the characters 0-9 and abcdef or ABCDEF are assumed to represent
 * the 16 levels in hex. 
 */
public class Frame implements BLImage {
	int width;
	int height;
	int bits;
	int duration;
	BufferedImage img;
	int rowCount;
	
	/**
	 * Creates a new Frame.
	 */
	public Frame(int width, int height, int bits, int duration) {
		img = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);
		this.width = width;
		this.height = height;
		this.bits = bits;
		this.duration = duration;
		rowCount = 0;
	}
	
	/**
	 * Adds a row.
	 */
	public void addRow(String rowChars) {
		int len = rowChars.length();
		if(img.getWidth() < len) {
			len = img.getWidth();
		}
		// find the magic maxval scale factor
		int maxVal = 15;
		if(bits == 3) maxVal = 7;
		if(bits == 2) maxVal = 3;
		if(bits == 1) maxVal = 1;
		double scaleFactor = 255.0 / maxVal;

		for(int i = 0; i < len; i ++) {
			int val = 0;
			try {
				val = (byte) ((int) (Integer.parseInt(rowChars.substring(i, i + 1), 16) * 
						scaleFactor) & 0xff);
			} catch(NumberFormatException e) {
				System.err.println("i: " + i);
				System.err.println("bad pixel data: " + rowChars + " - [" + rowChars.substring(i, i + 1) + "]");
			}
			img.setRGB(i, rowCount, (val << 24) | (val << 16) | val);
		}
		rowCount ++;
	}

	public void fillBufferedImage(BufferedImage target) {
		Graphics2D g2 = (Graphics2D)target.getGraphics();
		g2.drawImage(img, 0, 0, null);
	}
	
	public int getImageHeight() {
		return height;
	}

	public int getImageWidth() {
		return width;
	}
	
	public int getDuration() {
		return duration;
	}
}
