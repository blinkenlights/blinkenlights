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

package de.blinkenlights.bmix.network;

import java.awt.image.BufferedImage;
import java.io.IOException;
import java.util.List;
import java.util.concurrent.Semaphore;
import java.util.logging.Level;
import java.util.logging.Logger;

import de.blinkenlights.bmix.mixer.Layer;
import de.blinkenlights.bmix.protocol.BLFramePacket;
import de.blinkenlights.bmix.protocol.BLPacket;

/**
 * This class is thread that receives packets continuously. Every time a new
 * packet is received, this thread interrupts the notifyThread.
 */
public class BLPacketReceiverThread extends Thread {
	private final BLPacketReceiver receiver;
	private BLFramePacket latestFramePacket;
	private final Semaphore semaphore;
	private BufferedImage layerImage;
	private final long timeout;
	private long lastPacketReceiptTime;

	private final static Logger logger = Logger.getLogger(BLPacketReceiverThread.class.getName());
	/**
	 * The minimum interval between heartbeat packet sends, in milliseconds.
	 */
	private final int heartBeatSendInterval = 1000;
	/**
	 * The number of milliseconds since the epoch at which the most recent
	 * heartbeat packet was sent.  This is not immune to system clock adjustments.
	 */
	private long lastHeartBeatSendTime = 0;
	private boolean stopRequested;

	/**
	 * Creates a new thread that receives packets continuously. Every time a new
	 * packet is received, this thread interrupts the notifyThread.
	 * 
	 * @param receiver
	 *            The {@link BLPacketReceiver} to receive data from.
	 * @param sempaphore
	 *            Every time a packet is received, this receiver thread will
	 *            release on this semaphore.
	 * @param timeout
	 *            Number of milliseconds allowed since the last packet until
	 *            {@link #updateTargetLayers(List)} renders full transparency
	 *            instead of the last packet's image.
	 */
	public BLPacketReceiverThread(BLPacketReceiver receiver,
			Semaphore sempaphore, long timeout) {
		this.receiver = receiver;
		this.semaphore = sempaphore;
		this.timeout = timeout;
		if (receiver == null) {
			throw new NullPointerException("Null receiver not allowed");
		}
	}

	/**
	 * Closes the associated receiver and terminates this thread.
	 */
	public synchronized void close() {
	    stopRequested = true; 
	}
	
	public synchronized boolean isStopRequested() {
		return stopRequested;
	}
	
	@Override
	public void run() {
		try {
			while (!isStopRequested()) {
				long now = System.currentTimeMillis();
				if (now > lastHeartBeatSendTime + heartBeatSendInterval) {
					lastHeartBeatSendTime = now;
					try {
						receiver.sendHeartBeat();
					} catch (IOException e) {
						logger.log(Level.WARNING,"couldn't send a heartbeat packet",e);
					}
				}
				BLPacket packet = receiver.receive();
				if (packet instanceof BLFramePacket) {
					lastPacketReceiptTime = System.currentTimeMillis();
					BLFramePacket framePacket = (BLFramePacket) packet;
					setLatestPacket(framePacket);
				}
			}
		} finally {
			receiver.close();
		}
		logger.info("thread "+getName()+" stopping...");
	}

	/**
	 * Sets the latest packet, issuing a warning to System.err if the previously
	 * received frame packet has not yet been picked up.
	 */
	private synchronized void setLatestPacket(BLFramePacket received) {
		if (latestFramePacket != null) {
			logger.warning("discarding received packet that hasn't been picked up yet (port "
							+ receiver.port + ")");
		} else {
			semaphore.release();
		}
		latestFramePacket = received;
	}

	/**
	 * Returns the latest frame packet recieved if it has not already been
	 * picked up. Returns null otherwise.
	 * <p>
	 * Calling this method "clears the outbox" so to speak, so if you call it
	 * lots of times in a row, you will get null on subsequent tries, at least
	 * until another packet comes in on the receiver.
	 */
	public synchronized BLFramePacket retrieveLatestPacket() {
		BLFramePacket retval = latestFramePacket;
		latestFramePacket = null;
		return retval;
	}

	public BLPacketReceiver getReceiver() {
		return receiver;
	}

	public void updateTargetLayers(List<Layer> layersForReceiver) {
		BLFramePacket fp = retrieveLatestPacket();
		if (fp != null) {
			for (Layer l : layersForReceiver) {
				if (layerImage == null
						|| fp.getImageWidth() != layerImage.getWidth()
						|| fp.getImageHeight() != layerImage.getHeight()) {
					layerImage = new BufferedImage(fp.getImageWidth(), fp
							.getImageHeight(), BufferedImage.TYPE_INT_ARGB);
				}
				fp.fillBufferedImage(layerImage);
				l.updateImage(layerImage, receiver.getInputCropOffset());
			}
		} else if (System.currentTimeMillis() > lastPacketReceiptTime + timeout) {
			for (Layer l : layersForReceiver) {
				l.clearImageToTransparent();
			}
		}
	}
}
