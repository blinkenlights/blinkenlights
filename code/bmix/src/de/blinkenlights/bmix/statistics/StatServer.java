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

package de.blinkenlights.bmix.statistics;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectOutputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.atomic.AtomicReference;
import java.util.logging.Logger;


public class StatServer implements Runnable {
	private static final Logger logger = Logger.getLogger(StatServer.class.getName());

	public static final int STAT_PORT = 4892;
	ServerSocket serverSocket;
	List<Socket> clientSockets;

//	private FrameStatistics lastFrameStatistic; 
	private AtomicReference<FrameStatistics> lastFrameStatistic = new AtomicReference<FrameStatistics>();
	
	/**
	 * Creates a status server.
	 * 
	 * @throws IOException on error 
	 */
	public StatServer() throws IOException {
		serverSocket = new ServerSocket(STAT_PORT);
		serverSocket.setSoTimeout(100);
		clientSockets = Collections.synchronizedList(new ArrayList<Socket>());
	}
	
	
	public void run() {
		while(true) {
			try {
				try {
					Socket clientSocket = serverSocket.accept();
					clientSocket.setSoTimeout(1000);
					logger.info("Stats client accepted from: " + clientSocket.getInetAddress().getHostAddress());
					logger.info("Stats server has: " + clientSockets.size() + " clients");
					clientSockets.add(clientSocket);
				} catch (SocketTimeoutException e) {
					// timeouts are normal, no worries!
				}
				sendToClients(getFrameStatistic());
			} catch (IOException e) {
				logger.warning(e.getMessage());
			}
		}
	}
	
	public void putFrameStatistic(FrameStatistics stats) {
		lastFrameStatistic.set(stats);
	}
	
	private FrameStatistics getFrameStatistic() {
		return lastFrameStatistic.get();
	}
	
	private void sendToClients(FrameStatistics frameStats) throws IOException {
		// don't waste time if we don't have any clients
		if(clientSockets.size() == 0) {
			return;
		}
		
		ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
		ObjectOutputStream outputStream = new ObjectOutputStream(byteStream);
		outputStream.writeObject(frameStats);
		outputStream.flush();
		outputStream.close();
		final byte[] byteArray = byteStream.toByteArray();
		
		for (Iterator<Socket> iter = clientSockets.iterator(); iter.hasNext();) {
			Socket clientSocket = iter.next();
			try {
				OutputStream clientOut = clientSocket.getOutputStream();
				clientOut.write(byteArray);
				clientOut.flush();
			} catch(IOException e) {
				logger.warning("error sending to client: " + e.getMessage() + " - removing it!");
				iter.remove();
				logger.info("Stats server has: " + clientSockets.size() + " clients");
			}
		}
	}
}
