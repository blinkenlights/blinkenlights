package de.blinkenlights.bmix.movie;

import java.awt.Dimension;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.io.OutputStream;
import java.util.Map;

/**
 * Outputs a BML (Blinkenlights Markup Language) from a sequence
 * of BLImage frames.
 * 
 * @author dfraser
 *
 */
public class BMLOutputStream extends OutputStream {

	private final OutputStream out;
	private final int millisPerFrame;
	private final Dimension size;
	private final int bpp;
	private boolean closed = false;
	private final Map<String, String> headerData;

	/**
	 * Opens a BML Output Stream and writes an appropriate header to the stream.
	 * 
	 * @param out the output stream into which the BML will be written
	 * @param fps the frames per second of the movie
	 * @param size the size of each frame
	 * @param bpp the bits-per-pixel of each frame in the movie.  8bpp and 4bpp are supported.
	 * @param headerData key/value data for the Header element of the BML file.  
	 * Each entry in the map will be turned into an XML element.
	 * @throws IOException if the wrapped output stream does.
	 */
	public BMLOutputStream(OutputStream out, int fps, Dimension size, int bpp, Map<String,String> headerData) throws IOException {
		this.out = out;
		this.headerData = headerData;
		this.millisPerFrame = 1000 / fps;
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
			String headerValue = headerData.get(headerKey);
			
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
				System.err.println("unsupported header found in map: "+headerKey);
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
		if (closed) {
			throw new IllegalStateException("attempt to write to closed stream");
		}
		if (image.getHeight() != size.height || image.getWidth() != size.width) {
			throw new IllegalArgumentException("Image was incorrect size for movie; expected ("+size.width+"x"+size.height+") but got ("+image.getWidth()+"x"+image.getHeight()+")");			
		}
		StringBuilder outStr = new StringBuilder();
		outStr.append("<frame duration=\""+millisPerFrame+"\">\n");
		for (int i = 0; i < image.getHeight(); i++) {
			outStr.append("<row>");
			for (int j = 0; j < image.getWidth(); j++) {
				// keep the red channel only
				int r = (image.getRGB(j, i) & 0x00ff0000) >> 16;
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
	}
	
	@Override
	public void write(int arg0) throws IOException {
		throw new UnsupportedOperationException("this stream can only write BLImages");
	}

	/**
	 * Writes the BML footer, closes this stream, and closes the wrapped stream.
	 * No further data should be written, since the footer has been written to the file.
	 */
	@Override
	public void close() throws IOException {
		super.close();
		writeFooter();
		out.flush();
		out.close();
		closed = true;
	}
}
