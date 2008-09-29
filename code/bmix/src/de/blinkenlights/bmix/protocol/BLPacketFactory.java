package de.blinkenlights.bmix.protocol;

import java.awt.Color;
import java.net.DatagramPacket;

import de.blinkenlights.bmix.network.BLPacketReceiver.AlphaMode;

/**
 * This class is a BLPacketFactory which can take a packet of network
 * bytes and figure out the packet type, and then return a BLPacket of
 * the correct type. It checks sanity of various values before creating
 * a new packet object.
 */
public class BLPacketFactory {	

	/**
	 * Creates a new BLPacket from raw bytes.
	 * 
	 * @param data the packet raw data to parse
	 * @param len the data length
	 * @throws BLPacketException if there is an error
	 */
	public static BLPacket parse(DatagramPacket packet, AlphaMode alphaMode,
	        Color transparentColour, Color shadowColor) throws BLPacketException {
	    byte data[] = packet.getData();
	    int len = packet.getLength();
		// length is invalid
		if(len > data.length || len < 0 || len < 4) {
			throw new BLPacketException("len is invalid");
		}
		// MCU setup packet - 0x2342feed
		if(data[0] == 0x23 && data[1] == 0x42 && 
				data[2] == (byte)0xfe && data[3] == (byte)0xed) {
			if(len < 16) {
				throw new BLPacketException("MCU_SETUP packet must have length >= 16 bytes");
			}
			int id = (int)(data[4] & 0xff);
			int height = ((int)(data[8] & 0xff) << 8) | (int)(data[9] & 0xff);
			int width = ((int)(data[10] & 0xff) << 8) | (int)(data[11] & 0xff);
			int channels = ((int)(data[12] & 0xff) << 8) | (int)(data[13] & 0xff);
			int pixels = ((int)(data[14] & 0xff) << 8) | (int)(data[15] & 0xff);
			if(height < 1 || width < 1 || channels < 1 || pixels < 1) {
				throw new BLPacketException("MCU_SETUP packet has 0 dimensional size, 0 channels or 0 ports");
			}
			if(len != (pixels + 16)) {
				throw new BLPacketException("MCU_SETUP packet size wrong - got: " + len +
						" - expect: " + (pixels + 16));
			}
			byte pixelData[] = new byte[pixels];
			System.arraycopy(data, 16, pixelData, 0, len - 16);
			return new BLSetupPacket(id, width, height, channels, pixels, pixelData);
		}
		
		// MCU control packet - 0x23542667
		else if(data[0] == 0x23 && data[1] == 0x54 &&
				data[2] == 0x26 && data[3] == 0x67) {
			if(len < 8) {
				throw new BLPacketException("MCU_DEVCTRL packet must have length >= 8 bytes");
			}
			int pixels = ((int)(data[4] & 0xff) << 8) | (int)(data[5] & 0xff);
			if(pixels < 1) {
				throw new BLPacketException("MCU_DEVCTRL packet has 0 pixels");
			}
			if(len != (pixels + 8)) {
				throw new BLPacketException("MCU_DEVCTRL packet size wrong - got: " + len +
						" - expect: " + (pixels + 8));
			}
			byte pixelData[] = new byte[pixels];
			System.arraycopy(data, 8, pixelData, 0, len - 8);
			return new BLControlPacket(pixelData);		
		}
		
		// heartbeat packet - 0x42424242
		else if(data[0] == 0x42 && data[1] == 0x42 &&
				data[2] == 0x42 && data[3] == 0x42) {
			if(len < 6) {
				throw new BLPacketException("HEARTBEAT packet must have length = 6");
			}
			int version = ((int)(data[4] & 0xff) << 8) | (int)(data[5] & 0xff);
			return new BLHeartbeatPacket(version, packet.getAddress().getHostAddress(), packet.getPort());
		}
		
		// MCU frame packet - 0x23542666
		else if(data[0] == 0x23 && data[1] == 0x54 &&
				data[2] == 0x26 && data[3] == 0x66) {
			if(len < 12) {
				throw new BLPacketException("MCU_FRAME packet must length >= 12 bytes");
			}
			int height = ((int)(data[4] & 0xff) << 8) | (int)(data[5] & 0xff);
			int width = ((int)(data[6] & 0xff) << 8) | (int)(data[7] & 0xff);
			int channels = ((int)(data[8] & 0xff) << 8) | (int)(data[9] & 0xff);
			int maxval = ((int)(data[10] & 0xff) << 8) | (int)(data[11] & 0xff);  
			if(height < 1 || width < 1 || channels < 1) {
				throw new BLPacketException("MCU_FRAME packet has 0 dimensional size or 0 channels");
			}
			if(len != ((height * width * channels) + 12)) {
				throw new BLPacketException("MCU_FRAME packet size wrong - got: " + len +
						" - expect: " + ((height * width * channels) + 12));
			}
			if (channels != 1) {
				throw new BLPacketException("MCU_FRAME Unsupported channels value: " + channels);
			}
			byte pixels[] = new byte[len - 12];
	
			// find the magic maxval scale factor
			double scaleFactor = 255.0 / maxval;
			
			for (int i = 0; i < pixels.length; i++) {
				pixels[i] = (byte) ((int) (data[i+12] * scaleFactor) & 0xff);
			}
			
			return new BLFramePacket(width, height, pixels, alphaMode, transparentColour, shadowColor);
		}
		
		// legacy BL frame packet - 0xdeadbeef 
		else if(data[0] == (byte)0xde && data[1] == (byte)0xad &&
				data[2] == (byte)0xbe && data[3] == (byte)0xef) {
			if(len < 12) {
				throw new BLPacketException("LEGACY_FRAME must be length >= 12 bytes");
			}
			int width = ((int)(data[8] & 0xff) << 8) | (int)(data[9] & 0xff);
			int height = ((int)(data[10] & 0xff) << 8) | (int)(data[11] & 0xff);
			if(height < 1 || width < 1) {
				throw new BLPacketException("LEGACY_FRAME packet has 0 dimensional size");
			}
			byte pixels[] = new byte[len - 12];
			
			for (int i = 0; i < pixels.length; i++) {
				if (data[i+12] != 0) {
					pixels[i] = 1;
				}
			}
			return new BLFramePacket(width, height, pixels, alphaMode, transparentColour, shadowColor);
		}
		
		// legacy greyscale BL frame packet - 0xfeedbeef
		else if(data[0] == (byte)0xfe && data[1] == (byte)0xed &&
				data[2] == (byte)0xbe && data[3] == (byte)0xef) {
			if(len < 12) {
				throw new BLPacketException("LEGACY_FRAME must be length >= 12 bytes");
			}
			int width = ((int)(data[8] & 0xff) << 8) | (int)(data[9] & 0xff);
			int height = ((int)(data[10] & 0xff) << 8) | (int)(data[11] & 0xff);
			if(height < 1 || width < 1) {
				throw new BLPacketException("LEGACY_FRAME packet has 0 dimensional size");
			}
			byte pixels[] = new byte[len - 12];
			System.arraycopy(data, 12, pixels, 0, len - 12);
			return new BLFramePacket(width, height, pixels, alphaMode, transparentColour, shadowColor);
		}	
		return null;
	}
}
