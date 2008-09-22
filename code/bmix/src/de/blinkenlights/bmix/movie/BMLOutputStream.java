package de.blinkenlights.bmix.movie;

import java.awt.Dimension;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.io.OutputStream;

import de.blinkenlights.bmix.mixer.BLImage;

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

	/**
	 * Opens a BML Output Stream and writes an appropriate header to the stream.
	 * 
	 * @param out the output stream into which the BML will be written
	 * @param fps the frames per second of the movie
	 * @param size the size of each frame
	 * @param bpp the bits-per-pixel of each frame in the move
	 * @throws IOException if the wrapped output stream does.
	 */
	public BMLOutputStream(OutputStream out, int fps, Dimension size, int bpp) throws IOException {
		this.out = out;
		this.millisPerFrame = 1000 / fps;
		this.size = size;
		this.bpp = bpp;
		writeHeader();
	}

	private void writeHeader() throws IOException {
		StringBuilder outStr = new StringBuilder();
		outStr.append("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
		outStr.append("<blm width=\""+size.getWidth()+"\" height=\""+size.getHeight()+"\" bits=\""+bpp+"\" channels=\"1\">\n");
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
	public void writeFrame(BufferedImage image) {
		if (closed) {
			throw new IllegalStateException("attempt to write to closed stream");
		}
		if (image.getHeight() != size.height || image.getWidth() != size.width) {
			throw new IllegalArgumentException("image was incorrect size for movie; expected ("+size.getHeight()+"x"+size.getWidth()+") but got ("+image.getHeight()+"x"+image.getWidth()+")");			
		}
		StringBuilder outStr = new StringBuilder();
		outStr.append("<frame duration="+millisPerFrame+">\n");
		for (int i = 0; i < image.getHeight(); i++) {
			outStr.append("<row>");
			for (int j = 0; j < image.getWidth(); j++) {
				// keep the red channel only
				int r = (image.getRGB(i, j) & 0x00ff0000) >> 16;
				// clamp the bpp
				r = r >> (8 - bpp);
				outStr.append(String.format("%x", r));
			}
			outStr.append("</row>\n");
		}
		outStr.append("</frame>\n");
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
		out.close();
		closed = true;
	}
}
