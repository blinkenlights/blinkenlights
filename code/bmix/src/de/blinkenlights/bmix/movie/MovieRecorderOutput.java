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
import java.awt.Rectangle;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collections;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.zip.GZIPOutputStream;

import de.blinkenlights.bmix.mixer.BLImage;
import de.blinkenlights.bmix.mixer.BLImageViewport;
import de.blinkenlights.bmix.mixer.Output;
import de.blinkenlights.bmix.mixer.AbstractOutput.PacketType;
import de.blinkenlights.bmix.network.HostAndPort;

public class MovieRecorderOutput implements Output {
	
    private static final Logger logger = Logger.getLogger(MovieRecorderOutput.class.getName());

	private int screens;
	private Rectangle bounds;
	private int bpp;
	private final File destinationDirectory;
	private final String baseFilename;
	private BLImageViewport viewport;
	private long lastFrameTime;
	private int fileHour;
	
	private final Map<String, String> headerData = new HashMap<String, String>();

	private static final SimpleDateFormat filenameDate = new SimpleDateFormat(
			"yyyyMMdd'T'HHmmss");

	private BMLOutputStream out;
	private final BLImage source;
	private final boolean gzip;

	public MovieRecorderOutput(BLImage source, File destinationDirectory,
			String baseFilename, boolean gzip) throws IOException {
		this.source = source;
		this.destinationDirectory = destinationDirectory;
		this.baseFilename = baseFilename;
		this.gzip = gzip;
 	}

	private synchronized void roll() throws IOException {
		if (out != null) {
			out.flush();
			out.close();
		}
		String extension;
		if (gzip) {
			extension =".bml.gz";
		} else {
			extension = ".bml";
		}
		File outputFile = new File(destinationDirectory, baseFilename +"-"
				+ filenameDate.format(new Date()) + extension);

		OutputStream baseOutputStream;
		if (gzip) {
			baseOutputStream = new GZIPOutputStream(new FileOutputStream(
					outputFile));
		} else {
			baseOutputStream = new FileOutputStream(outputFile);
		}
		
		logger.info("rolling recording file, new file: "+outputFile.getAbsolutePath());

		out = new BMLOutputStream(baseOutputStream, 0, new Dimension(
				bounds.width, bounds.height), bpp, headerData);
	}

	public void addScreen(Rectangle bounds, int bpp, int screenId) {
		if (this.screens > 0) {
			throw new UnsupportedOperationException(
					"Movie recorder supports only one screen");
		}
		this.bounds = bounds;
		this.bpp = bpp;
		viewport = new BLImageViewport(source, bounds, bpp, 0);
		// we have a screen now, kick off the file!
		try {
			roll();
		} catch (IOException e) {
			logger.log(Level.SEVERE,"unable to open roll over log file: ",e);
		}
		this.screens++;
	}

	public void close() {
		try {
			out.close();
		} catch (IOException e) {
			logger.log(Level.WARNING, "unable to close recorder output stream",e);
		}		
	}

	public List<HostAndPort> getDestinations() {
		return new ArrayList<HostAndPort>();
	}

	public long getLastSendTime() {
		return lastFrameTime;
	}

	public long getMinSendInterval() {
		return 0;
	}

	public PacketType getPacketType() {
		return null;
	}

	public List<BLImageViewport> getViewports() {
		return Collections.singletonList(viewport);
	}

	public void send() throws IOException {
		
		int thisHour = new GregorianCalendar().get(Calendar.HOUR_OF_DAY);
		if (thisHour != fileHour) {
			roll();
		}
		
		fileHour = thisHour;
		BufferedImage bi = new BufferedImage(viewport.getImageWidth(), viewport.getImageHeight(), BufferedImage.TYPE_INT_ARGB);
		viewport.fillBufferedImage(bi);
		out.writeFrame(bi);
		lastFrameTime = System.currentTimeMillis();
	}
}
