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
package de.blinkenlights.game;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.util.concurrent.Semaphore;
import java.util.logging.Level;
import java.util.logging.Logger;

import de.blinkenlights.bvoip.blt.BLTCommand;
import de.blinkenlights.bvoip.blt.BLTCommand.CommandType;

/**
 * A client to the Blinkenlights Telephony server.
 */
class BLTClient implements UserInputSource, Runnable {

	Logger logger = Logger.getLogger(BLTClient.class.getName());

	private final InetAddress host;
	private final int port;
	private final String did;

	private Character lastKeystroke = null;

	private Semaphore gameStart = new Semaphore(0);

	/**
	 * The BLT channel we are associated with. When we are not on a
	 * call, this value is -1.
	 */
	private int bltChannel = -1;

	public BLTClient(InetAddress host, int port, String did) {
		this.host = host;
		this.port = port;
		this.did = did;
		try {
			socket = new DatagramSocket();
			socket.setSoTimeout(1000); 
		} catch (SocketException e) {
			throw new RuntimeException("unable to set up socket for pinger thread",e);
		}
	}

	public void start() {
		if (isRunning()) {
			throw new IllegalStateException("This phone client is already running");
		}
		new Thread(this).start();
	}

	public void stop() {
		setStopRequested(true);
	}

	private synchronized void setKeystroke(Character keyStroke) {
		lastKeystroke = keyStroke;
	}

	public synchronized Character getKeystroke() {
		Character keyStroke = lastKeystroke;
		lastKeystroke = null;
		return keyStroke;
	}

	public void waitForGameStart() throws InterruptedException {
		gameStart.acquire();
	}

	public void gameEnding() {
		try {
			sendHangup(bltChannel);
		} catch (IOException e) {
			logger.log(Level.WARNING,"error sending hangup",e);
		} finally {
		    gameStart.drainPermits();
		}
		bltChannel = -1;
	}

	public synchronized boolean isUserPresent() {
		return bltChannel != -1;
	}

	public InetAddress getHost() {
		return host;
	}

	public int getPort() {
		return port;
	}

	public String getDid() {
		return did;
	}




	private final int timeWithoutChannelInformationBeforeWeReRegisterMillis = 15000;
	private boolean running;
	private final int heartbeatIntervalMillis = 15000;
	private long lastHeartBeatSent = 0;
	private boolean stopRequested = false;
	private long lastChannelInfoTimeMillisSinceEpochGMT;
	private DatagramSocket socket;


	public void run() {
		setRunning(true);
		try {

			while (!isStopRequested()) {
				if (System.currentTimeMillis() - lastChannelInfoTimeMillisSinceEpochGMT > timeWithoutChannelInformationBeforeWeReRegisterMillis) {
					// time to try registering
					sendRegister();
				}

				sendHeartbeat(socket);

				try {
					byte[] buf = new byte[1500];
					DatagramPacket inPacket = new DatagramPacket(buf, buf.length);
					socket.receive(inPacket);	
					BLTCommand c = BLTCommand.parsePacket(new String(buf, "ISO8859-1"));
					//logger.info("got command: "+c);
					processCommand(c);
				} catch (SocketTimeoutException f) {
					// timeout, no big deal
					logger.finest("listen socket timeout");
				} catch (IOException e) {
					logger.log(Level.WARNING, "Receive failed. Will try again.", e);
					Thread.sleep(1000);
				}

			}
		} catch (Exception ex) {
			logger.log(Level.SEVERE, "Pinger thread is dying!", ex);
		} finally {
			setRunning(false);
			setStopRequested(false);
		}
	}

	private void sendHeartbeat(DatagramSocket socket) throws IOException {
		if (System.currentTimeMillis() - lastHeartBeatSent > heartbeatIntervalMillis) {
			logger.finest("Sending heartbeat to "+host+":"+port);
			byte[] heartbeat = new BLTCommand(0, CommandType.HEARTBEAT).getNetworkBytes();
			DatagramPacket heartbeatPacket = new DatagramPacket(
					heartbeat, heartbeat.length, host, port);
			socket.send(heartbeatPacket);
			lastHeartBeatSent = System.currentTimeMillis();
		}
	}

	private void sendPlaybackground(String context) throws IOException {
		logger.fine("Sending playbackground to "+host+":"+port);
		BLTCommand command = new BLTCommand(bltChannel, CommandType.PLAYBACKGROUND, context);
		byte[] buf = command.getNetworkBytes();
		DatagramPacket packet = new DatagramPacket(
				buf, buf.length, host, port);
		socket.send(packet);
	}

	private void sendRegister() throws IOException {
		logger.info("Sending registration to "+host+":"+port);
		byte[] register = new BLTCommand(0, CommandType.REGISTER).getNetworkBytes();
		DatagramPacket registerPacket = new DatagramPacket(
				register, register.length, host, port);
		socket.send(registerPacket);
	}

	private void sendAccept() throws IOException {
		if (bltChannel == -1) {
			logger.warning("no channel to accept!");
			return;
		}
		logger.info("Sending accept to "+host+":"+port);
		byte[] buf = new BLTCommand(bltChannel, CommandType.ACCEPT).getNetworkBytes();
		DatagramPacket packet = new DatagramPacket(
				buf, buf.length, host, port);
		socket.send(packet);
	}


	private void sendHangup(int channel) throws IOException {
		if (channel == -1) {
			logger.warning("no channel to hang up!");
			return;
		}
		logger.info("Sending hangup on channel "+channel+" to "+host+":"+port);
		byte[] hangup = new BLTCommand(channel, CommandType.HANGUP).getNetworkBytes();
		DatagramPacket hangupPacket = new DatagramPacket(
				hangup, hangup.length, host, port);
		socket.send(hangupPacket);
	}

	private void processCommand(BLTCommand c) throws IOException {
		if (c.getCommand() == CommandType.ONHOOK) {
			lastChannelInfoTimeMillisSinceEpochGMT = System.currentTimeMillis();
			if (c.getChannelNum() == bltChannel) {
				logger.info("Got user hangup. Disassociating with channel " + bltChannel);
				bltChannel = -1;
			}
		}
		else if (c.getCommand() == CommandType.SETUP) {
			String cid = c.getArgs().get(0);
			String dnid = c.getArgs().get(1);
			int channel = c.getChannelNum();
			logger.info("got call on channel "+channel+" for "+dnid+" from "+cid);

			// TODO: thread safety
			synchronized (this) {				
				if (dnid != null && dnid.equals(getDid())) {
					if (bltChannel == -1) {
						logger.info("this call is for our game, and the game is available");
						bltChannel = channel;
						sendAccept();
						gameStart.release();
					} 
					else {
						logger.info("got a call, but game is already in progress");
						sendHangup(channel);
					}
				}
			}

		}
		else if (c.getCommand() == CommandType.CONNECTED) {
			lastChannelInfoTimeMillisSinceEpochGMT = System.currentTimeMillis();
			// we don't really need to do anything with this
		}
		else if (c.getCommand() == CommandType.DTMF) {
			String digit = c.getArgs().get(0);
			logger.fine("got digit: "+digit);
			setKeystroke(Character.valueOf(digit.charAt(0)));
		}
		else if (c.getCommand() == CommandType.ERROR && c.getChannelNum() == bltChannel) {
			String message = c.getArgs().get(0);
			logger.info("got error from server about our channel: "+message);
			if ("not your call".equals(message)) {
				// hmm, I guess that wasn't our call!
				bltChannel = -1;
			}
			else if ("no call on this channel".equals(message)) {
				// hmm, i guess that wasn't our call either!
				bltChannel = -1;
			}
		}
	}

	
	
	private synchronized boolean isStopRequested() {
		return stopRequested ;
	}

	public synchronized void setStopRequested(boolean stopRequested) {
		this.stopRequested = stopRequested;
	}

	private synchronized void setRunning(boolean running) {
		this.running = running;
	}

	public synchronized boolean isRunning() {
		return running;
	}

	public void playBackgroundMusic(String musicName) {
		try {
			sendPlaybackground(musicName);
		} catch (IOException e) {
			logger.log(Level.WARNING,"couldn't start background music",e);
		}
		
	}

}

