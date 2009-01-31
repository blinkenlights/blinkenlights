package de.blinkenlights.bmix.directHardware;

import gnu.io.CommPort;
import gnu.io.CommPortIdentifier;
import gnu.io.NoSuchPortException;
import gnu.io.PortInUseException;
import gnu.io.SerialPort;
import gnu.io.UnsupportedCommOperationException;

import java.awt.image.BufferedImage;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import de.blinkenlights.bmix.mixer.OutputSender;

/**
 * Implements a driver for the sign. Can handle from 1 to 12 boards.
 * 
 * @author andrew
 * 
 * Protocol:
 * 
 * Frame format:
 * byte 0: SOH (0x01)
 * byte 1: frame data (64 bytes per board - 1 board at a time)
 * byte 2: EOT (0x04)
 * 
 * Byte stuffing: (sending control bytes in frame data)
 * - SOH = ESC + ESC_SOH
 * - EOT = ESC + ESC_EOT
 * - ESC = ESC + ESC_ESC
 * 
 * Control bytes:
 * - SOH = 0x01
 * - EOT = 0x04
 * - ESC = 0x1b
 * - ESC_SOH = 0x78
 * - ESC_EOT = 0x79
 * - ESC_ESC = 0x7a
 * 
 */
public class HacklabSignDriver implements OutputSender {
	// serial port 
	private String portName;
	private SerialPort port = null;
	private InputStream in = null;
	private OutputStream out = null;
	
	// protocol
	static final int SOH = 0x01;
	static final int EOT = 0x04;
	static final int ESC = 0x1b;
	static final int ESC_SOH = 0x78;
	static final int ESC_EOT = 0x79;
	static final int ESC_ESC = 0x7a;
	
	// board settings
	static final int BOARD_W = 16;
	static final int BOARD_H = 32;
	int numBoards = 6;
	
	
	/**
	 * Creates a new SignDriver instance.
	 * 
	 * @param serialPortName the serial port name
	 * @param numBoards the number of boards on the sign matrix
	 */
	public HacklabSignDriver(String serialPortName) {
		this.portName = serialPortName;
		
		if(numBoards < 1 || numBoards > 12) {
			throw new IllegalArgumentException("numBoards is out of range: " + numBoards);
		}

		// open the serial port
        CommPortIdentifier portIdentifier;
		try {
			portIdentifier = CommPortIdentifier.getPortIdentifier(serialPortName);
	        if(portIdentifier.isCurrentlyOwned()) {
	            System.err.println("Error: Port is currently in use: " +
	            		serialPortName);
	            System.exit(-1);
	        }
	        else {
	            CommPort commPort = portIdentifier.open(this.getClass().getName(),2000);
	            if(commPort instanceof SerialPort) {
	                port = (SerialPort) commPort;
	                port.setSerialPortParams(57600,
	                		SerialPort.DATABITS_8,
	                		SerialPort.STOPBITS_1,
	                		SerialPort.PARITY_NONE);
	                port.setFlowControlMode(SerialPort.FLOWCONTROL_NONE);
	                port.disableReceiveFraming();
	                port.disableReceiveTimeout();
	                port.disableReceiveThreshold();
	                in = port.getInputStream();
	                out = port.getOutputStream();
	            } else {
	                System.err.println("Port is not a serial port: " + serialPortName);
	            }
	        }
		} catch (NoSuchPortException e) {
			e.printStackTrace();
			System.exit(-1);
		} catch (PortInUseException e) {
			e.printStackTrace();
			System.exit(-1);
		} catch (UnsupportedCommOperationException e) {
			e.printStackTrace();
			System.exit(-1);
		} catch (IOException e) {
			e.printStackTrace();
			System.exit(-1);
		}
		System.out.println("SignDriver waiting 3s for startup...");
		try {
			Thread.sleep(3000);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
			
 	public void close() {
		port.close();
	}

	public void send(byte[] buf) throws IOException {
		send(buf, 0, buf.length);
	}

	public void send(byte[] pixels, int offset, int length) throws IOException {		
		// map pixels to sign formatted pixels
		byte buf[] = new byte[(pixels.length - 12) / 8];
		
		// load each board
		int outCount = 0;
		for(int board = 0; board < numBoards; board ++) {
			for(int row = 0; row < BOARD_H; row ++) {
				for(int partRow = 0; partRow < 2; partRow ++) {
					for(int pixel = 0; pixel < 8; pixel ++) {
						if(pixels[((row * (BOARD_W * numBoards)) + (board * BOARD_W) + (partRow * 8) + pixel) + 12] != 0) { 
							buf[outCount] |= (0x01 << pixel); 
						}
					}
					outCount ++;
				}
			}
		}
				
		// original 1 board converting code
//		for(int i = 0; i < buf.length; i ++) {
//			for(int j = 0; j < 8; j ++) {
//				if(pixels[pixelCount++] != 0) {
//					buf[i] |= (0x01 << j);
//				}				
//			}
//		}
		
//		printBuf(buf);

		
		byte txBuf[] = new byte[(buf.length * 2) + 2];
		int txBufCount = 0;
		txBuf[txBufCount++] = SOH;
		for(int i = 0; i < buf.length; i ++) {
			if(buf[i] == SOH) {
				txBuf[txBufCount++] = ESC;
				txBuf[txBufCount++] = ESC_SOH;
			}
			else if(buf[i] == EOT) {
				txBuf[txBufCount++] = ESC;
				txBuf[txBufCount++] = ESC_EOT;
			}
			else if(buf[i] == ESC) {
				txBuf[txBufCount++] = ESC;
				txBuf[txBufCount++] = ESC_ESC;
			}
			else {
				txBuf[txBufCount++] = buf[i];
			}
		}
		txBuf[txBufCount++] = EOT;

//		System.out.println("sending frame - data len: " + buf.length + " - txLen: " + txBufCount);
		
//		printBuf(txBuf, txBufCount);
		
		try {
			out.write(txBuf);
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	private void printBuf(byte buf[], int len) {
		for(int i = 0; i < len; i ++) {
			if(i % 16 == 0) {
				System.out.println("");
			}
			System.out.print(String.format("0x%02x ", buf[i]));
		}
		System.out.println("");
	}

	public int getPort() {
		return 0;
	}

	public String getAddress() {
		return portName;
	}
}
