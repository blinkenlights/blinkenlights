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

import java.awt.Dimension;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.io.OutputStream;
import java.util.Map;

import org.apache.commons.lang.StringEscapeUtils;

/**
 * Outputs a BML (Blinkenlights Markup Language) from a sequence
 * of BLImage frames.
 * 
 * @author dfraser
 *
 */
public class BMLOutputStream extends OutputStream {

	private final OutputStream out;
	private final boolean autoFps;
	private final long millisPerFrame;
	private final Dimension size;
	private long lastFrameTime;
	private final int bpp;
	private boolean closed = false;
	private final Map<String, String> headerData;
	private BufferedImage previousFrame;

	/**
	 * Opens a BML Output Stream and writes an appropriate header to the stream.
	 * 
	 * @param out the output stream into which the BML will be written
	 * @param fps the frames per second of the movie.  If fps is set to 0, the frame intervals will
	 * be automatically generated based on how often writeFrame() is called.  This allows
	 * the use of the output stream for live recording.  One full image will be buffered.
	 * @param size the size of each frame
	 * @param bpp the bits-per-pixel of each frame in the movie.  8bpp and 4bpp are supported.
	 * @param headerData key/value data for the Header element of the BML file.  
	 * Each entry in the map will be turned into an XML element.
	 * @throws IOException if the wrapped output stream does.
	 */
	public BMLOutputStream(OutputStream out, int fps, Dimension size, int bpp, Map<String,String> headerData) throws IOException {
		this.out = out;
		this.headerData = headerData;
		if (fps == 0) {
			autoFps = true;
			this.millisPerFrame = 0;
		} else {
			autoFps = false;
			this.millisPerFrame = 1000 / fps;
		}
		this.size = size;
		if (bpp != 4 && bpp != 8) {
			throw new IllegalArgumentException("BPP "+bpp+" is unsupported.  Only 4 and 8 are supported.");
		}
		this.bpp = bpp;
		
		writeHeader();
	}

	private void writeHeader() throws IOException {
		StringBuilder outStr = new StringBuilder();
		outStr.append("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
		outStr.append("<blm width=\""+size.width+"\" height=\""+size.height+"\" bits=\""+bpp+"\" channels=\"1\">\n");
		outStr.append("<header>\n");
		for (String headerKey : headerData.keySet()) {
			
			String headerValue = StringEscapeUtils.escapeXml(headerData.get(headerKey));
			
			if (headerKey.equals("title") && headerValue != null && headerValue.length() > 0) {
				outStr.append("<title>"+headerValue+"</title>\n");
			}
			else if (headerKey.equals("description") && headerValue != null && headerValue.length() > 0) {
				outStr.append("<description>"+headerValue+"</description>\n");
			}
			else if (headerKey.equals("author") && headerValue != null && headerValue.length() > 0) {
				outStr.append("<author>"+headerValue+"</author>\n");
			}
			else if (headerKey.equals("email") && headerValue != null && headerValue.length() > 0) {
				outStr.append("<email>"+headerValue+"</email>\n");
			}
			else if (headerKey.equals("url") && headerValue != null && headerValue.length() > 0) {
				outStr.append("<url>"+headerValue+"</url>\n");
			} else {
				System.err.println("unsupported or empty header found in map: "+headerKey);
			}
		}
		outStr.append("<creator>$Id$</creator>\n");
		outStr.append("</header>\n");
		
		byte[] bytes = outStr.toString().getBytes();
		out.write(bytes, 0, bytes.length);
	}
	
	private void writeFooter() throws IOException {
		StringBuilder outStr = new StringBuilder();
		outStr.append("</blm>\n");
		byte[] bytes = outStr.toString().getBytes();
		out.write(bytes, 0, bytes.length);
	}
	
	/**
	 * Writes a new frame to the stream. 
	 * @param image the BLImage containing the frame to write.
	 * 
	 * @throws IllegalArgumentException if the frame is the incorrect dimensions for the movie.
	 */
	public void writeFrame(BufferedImage image) throws IOException {
		BufferedImage outputImage;
		if (autoFps) {
			if (previousFrame == null) {
				// first frame, just buffer this one and return
				previousFrame = image;
				lastFrameTime = System.currentTimeMillis();
				return;
			}
 			outputImage = previousFrame;
		} else {
			outputImage = image;
		}
		
		if (closed) {
			throw new IllegalStateException("attempt to write to closed stream");
		}
		if (outputImage.getHeight() != size.height || outputImage.getWidth() != size.width) {
			throw new IllegalArgumentException("Image was incorrect size for movie; expected ("+size.width+"x"+size.height+") but got ("+outputImage.getWidth()+"x"+outputImage.getHeight()+")");			
		}
		
		StringBuilder outStr = new StringBuilder();
		if (autoFps) {
			long duration = System.currentTimeMillis() - lastFrameTime;
			outStr.append("<frame duration=\""+duration+"\">\n");
		} else {
			outStr.append("<frame duration=\""+millisPerFrame+"\">\n");
		}
		for (int i = 0; i < outputImage.getHeight(); i++) {
			outStr.append("<row>");
			for (int j = 0; j < outputImage.getWidth(); j++) {
				// keep the red channel only
				int r = (outputImage.getRGB(j, i) & 0x00ff0000) >> 16;
				if (bpp == 4) {
					r = r >> 4;
					outStr.append(String.format("%x", r));
				}
				else if (bpp == 8) {
					outStr.append(String.format("%02x", r));
				}
			}
			outStr.append("</row>\n");
		}
		outStr.append("</frame>\n");
		byte[] bytes = outStr.toString().getBytes();
		
		out.write(bytes, 0, bytes.length);
		if (autoFps) {
			previousFrame = image;
		}
		lastFrameTime = System.currentTimeMillis();
	}
	
	@Override
	public void write(int arg0) throws IOException {
		throw new UnsupportedOperationException("this stream can only write BLImages");
	}

	/**
	 * Writes the BML footer, closes this stream, and closes the wrapped stream.
	 * No further data should be written, since the footer has been written to the file.
	 * If we're in autofps mode, we have to flush the final frame out of the buffer.
	 */
	@Override
	public void close() throws IOException {
		super.close();
		if (autoFps) {
			// flush the last frame
			writeFrame(null);
		}
		writeFooter();
		out.flush();
		out.close();
		closed = true;
	}
}
