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

package de.blinkenlights.bmix.mixer;

import java.awt.image.BufferedImage;

/**
 * Instances of BLImage can provide a rectangle of pixel data. Each pixel has an
 * 8-bit alpha and an RGB value.
 */
public interface BLImage {
	
	/**
	 * Fills the passed BufferedImage with the content of this BLImage. If the given
	 * image is too small, the contents will be clipped, with the upper left corner as
	 * the anchor point. If the given image is too large, the extra pixels along the
	 * right and below will remain untouched.
	 * 
	 * @param target the buffered image to write into
	 */
	void fillBufferedImage(BufferedImage target);
	
	/**
	 * Returns the height in pixels of the content of this BLImage
	 */
	int getImageHeight();
	
	/**
	 * Returns the width in pixels of the content of this BLImage
	 */
	int getImageWidth();

}
