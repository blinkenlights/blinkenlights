/* 
 * This file is part of BMix.
 *
 *    BMix is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 * 
 *    BMix is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with BMix.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

package de.blinkenlights.bmix.movie;

import java.awt.Component;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.image.BufferedImage;

import javax.swing.Icon;

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
public class Frame implements BLImage, Icon {
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
			String pixelHexChar = rowChars.substring(i, i + 1);
            try {
				int packedValue = Integer.parseInt(pixelHexChar, 16);
                val = ((int) (packedValue * scaleFactor) & 0xff);
			} catch(NumberFormatException e) {
				System.err.println("i: " + i);
				System.err.println("bad pixel data: " + rowChars + " - [" + pixelHexChar + "]");
			}
			img.setRGB(i, rowCount, (0xff << 24) | (val << 16) | (val << 8) | val);
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

    public int getIconHeight() {
        return getImageHeight();
    }

    public int getIconWidth() {
        return getImageWidth();
    }

    public void paintIcon(Component c, Graphics g, int x, int y) {
        g.drawImage(img, x, y, null);
    }
	
	
}
