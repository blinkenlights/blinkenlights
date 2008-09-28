package processing.blinkenlights;

import java.io.IOException;

import processing.blinkenlights.AbstractFramePacket.AlphaMode;
import processing.core.PApplet;
import processing.core.PImage;

public class PacketSender {
	
	private final PApplet parent;
	private BLPacketSender sender = null;
	
	public PacketSender(PApplet parent, String host, int port) {
	    this.parent = parent;
	    parent.registerDispose(this);
	    try {
			sender = new BLPacketSender(host, port);
		} catch (IOException e) {
			parent.die("network error: "+e.getMessage());
			e.printStackTrace();
		}
	  }
	
	public void sendFrame() {
		if (sender != null) {
			PImage image = parent.get();
			
			int pixels = image.width * image.height;
			byte[] pixelData = new byte[pixels];
			for (int i = 0; i < pixels; i++) {
				pixelData[i] = (byte) (image.pixels[i] & 0x0000ff);
			}
			
			BLFramePacket packet = new BLFramePacket(image.width, image.height,
					pixelData, AlphaMode.OPAQUE, null);
			
			try {
				sender.send(packet.getNetworkBytes());
			} catch (IOException e) {
				parent.die("network error: "+e.getMessage());
				e.printStackTrace();
			}
		}
	}
	
	public void dispose() {
		sender.close();
	}
}
