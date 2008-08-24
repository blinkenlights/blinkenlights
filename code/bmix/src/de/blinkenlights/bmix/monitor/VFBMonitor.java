package de.blinkenlights.bmix.monitor;

import java.awt.AlphaComposite;
import java.awt.Rectangle;
import java.awt.image.BufferedImage;
import java.net.SocketException;

import de.blinkenlights.bmix.mixer.BLImage;
import de.blinkenlights.bmix.mixer.Layer;
import de.blinkenlights.bmix.network.BLPacketReceiver;
import de.blinkenlights.bmix.protocol.BLFramePacket;
import de.blinkenlights.bmix.protocol.BLPacket;

/**
 * A special monitor implementation that shows a virtual frame buffer.
 * This class exists mainly for testing purposes and as a demonstration
 * of how cool the API is.
 */
public class VFBMonitor extends Monitor {
	private BLPacketReceiver bpr;
	private Layer rootLayer;
	
	private Layer layer;
	private Layer layer2;
	private Layer layer3;

	public VFBMonitor() throws SocketException {
		super("VFBMonitor", 100, 100, 640, 480, true);
		this.bpr = new BLPacketReceiver(2323,null,null,0);
		
		rootLayer = new Layer(new Rectangle(0, 0, 96, 32), AlphaComposite.Src);
		
		layer = new Layer(
				new Rectangle(0, 0, rootLayer.getImageWidth(), rootLayer.getImageHeight()),
				AlphaComposite.getInstance(AlphaComposite.SRC_OVER, 1f));

		layer2 = new Layer(
				new Rectangle(15, 17, 18, 8),
				AlphaComposite.getInstance(AlphaComposite.SRC_OVER, .7f));

		layer3 = new Layer(
				new Rectangle(45, 17, 18, 8),
				AlphaComposite.getInstance(AlphaComposite.SRC_OVER, .7f));

		rootLayer.addLayer(layer);
		rootLayer.addLayer(layer2);
		rootLayer.addLayer(layer3);
		
	}

	@Override
	protected BLImage getNextImage() {
		for (;;) {
			BLPacket bp = bpr.receive();
			if (bp instanceof BLFramePacket) {
				BLImage blim = (BLFramePacket) bp;
				
				BufferedImage layerImage = new BufferedImage(18, 8, BufferedImage.TYPE_INT_ARGB);
				blim.fillBufferedImage(layerImage);

				layer.updateImage(layerImage);
				layer2.updateImage(layerImage);
				layer3.updateImage(layerImage);
				
				rootLayer.mixdown();
				
				return rootLayer;
			}
		}
	}

	public static void main(String[] args) throws Exception {
		VFBMonitor m = new VFBMonitor();
		m.run();
	}
}
