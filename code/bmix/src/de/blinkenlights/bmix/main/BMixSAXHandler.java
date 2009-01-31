/* 
 * Created on Oct 6, 2008
 *
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
package de.blinkenlights.bmix.main;

import java.awt.AlphaComposite;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Point;
import java.awt.Rectangle;
import java.io.File;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.xml.sax.Attributes;
import org.xml.sax.Locator;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import de.blinkenlights.bmix.directHardware.HacklabSignDriver;
import de.blinkenlights.bmix.mixer.DynamicOutput;
import de.blinkenlights.bmix.mixer.FixedOutput;
import de.blinkenlights.bmix.mixer.Layer;
import de.blinkenlights.bmix.mixer.Output;
import de.blinkenlights.bmix.mixer.OutputSender;
import de.blinkenlights.bmix.mixer.AbstractOutput.PacketType;
import de.blinkenlights.bmix.movie.MovieRecorderOutput;
import de.blinkenlights.bmix.network.BLPacketReceiver;
import de.blinkenlights.bmix.network.BLPacketSender;
import de.blinkenlights.bmix.network.BLPacketReceiver.AlphaMode;
import de.blinkenlights.bmix.util.FileFormatException;

public class BMixSAXHandler extends DefaultHandler {
	
	/**
	 * Document locator provided by SAX parser. Helps when reporting errors.
	 */
    private Locator locator;

    /**
     * Maps input IDs to the packet receivers responsible for those inputs.
     */
    private final Map<String, BLPacketReceiver> inputs = new LinkedHashMap<String, BLPacketReceiver>();
    
    /**
     * Mapping of inputs to the layers they feed. Inverse of {@link #layerSources}
     */
    private final Map<BLPacketReceiver, List<Layer>> layerInputs = new LinkedHashMap<BLPacketReceiver, List<Layer>>();

    /**
     * Mapping of layers to inputs that feed them. Inverse of {@link #layerInputs}
     */
    private final Map<Layer, BLPacketReceiver> layerSources = new HashMap<Layer, BLPacketReceiver>();
     
    /**
     * The virtual default layer that the mixdown will mix down to.
     * The top-level layers defined in the file are direct children of this layer.
     * Gets created by the SAX handler.
     */
    private Layer rootLayer;

    /**
     * The current layer we're configuring/populating.
     */
    private Layer currentLayer;

    /**
     * The current output we're configuring/populating.
     */
    private Output currentOutput;

    /**
     * The input layer we're configuring/populating.
     */
    private BLPacketReceiver currentInput;
    
    private BMixSession session;

    private long maxFrameInterval;
    
    private List<Output> outputs = new ArrayList<Output>();

    @Override
	public void setDocumentLocator(Locator locator) {
        this.locator = locator;
	}
	
	public BMixSession getConfiguration() {
	    if (session == null) {
	        session = new BMixSession(rootLayer, layerInputs, layerSources, outputs, maxFrameInterval);
	    }
        return session;
    }

    /**
	 * Handles a new element.
	 * 
	 * @param uri namespace URI
	 * @param localName the local name (without prefix)
	 * @param qName the qualified name (with prefix)
	 * @param attributes the attributes for this element 
	 */
	@Override
	public void startElement(String uri, String localName, String qName, Attributes attributes)
	throws SAXException {
	    try {
	        if (qName.equals("bmix")) {
	            String version = attributes.getValue("version");
	            Dimension vmatrixSize = new Dimension();
	            vmatrixSize.width = Integer.parseInt(attributes.getValue("vmatrix-width"));
	            vmatrixSize.height = Integer.parseInt(attributes.getValue("vmatrix-height"));
	            maxFrameInterval = Long.parseLong(attributes.getValue("max-frame-interval"));
	            if (!"1.0".equals(version)) {
	                throw new FileFormatException(
	                        "Unsupported bmix config version: " + version,
	                        locator.getLineNumber(), locator.getColumnNumber());
	            }
	            rootLayer = new Layer(
	                    new Rectangle(0, 0, vmatrixSize.width, vmatrixSize.height),
	                    AlphaComposite.Src);
	            currentLayer = rootLayer;

	        } else if (qName.equals("input")) {
	            String id = attributes.getValue("id");
	            String listenAddr = attributes.getValue("listen-addr");
	            String listenPort = attributes.getValue("listen-port");
	            String heartBeatDestAddrString = attributes.getValue("heartbeat-dest-addr");
	            AlphaMode alphaMode = AlphaMode.forCode(attributes.getValue("alpha-mode"));
	            Color transparentColour = Color.decode(attributes.getValue("chroma-key-colour"));
	            String shadowColourCode = attributes.getValue("shadow-colour");
	            Color shadowColour = null;
	            if (shadowColourCode != null) {
	            	// have to use Long so the result is clean for 32-bit signed
	            	int argb = (int) (Long.decode(shadowColourCode) & 0xffffffffL);
					shadowColour = new Color(argb, true);
	            }
	            InetAddress heartBeatDestAddr = null;
	            if (heartBeatDestAddrString != null) {
	            	heartBeatDestAddr = InetAddress.getByName(heartBeatDestAddrString);
	            } 
	            int heartBeatDestPort = Integer.parseInt(attributes.getValue("heartbeat-dest-port"));
	            int inputTimeout = Integer.parseInt(attributes.getValue("timeout"));
	            
	            String cropXString = attributes.getValue("cropx");
	            String cropYString = attributes.getValue("cropy");
	            
	            Point cropOffset = null;
	            if (cropXString != null && cropYString != null) {
	            	cropOffset = new Point(
	            			Integer.parseInt(cropXString),
	            			Integer.parseInt(cropYString));
	            }
	            
	            BLPacketReceiver receiver = 
	                new BLPacketReceiver(id,
	                        Integer.parseInt(listenPort),
	                        InetAddress.getByName(listenAddr),
	                        heartBeatDestAddr, heartBeatDestPort, alphaMode,
	                        transparentColour, shadowColour, inputTimeout, 
	                        cropOffset);
	            inputs.put(id, receiver);
	            currentInput = receiver;

	        } else if (qName.equals("relay-target")) {
                String destAddr = attributes.getValue("dest-addr");
                int destPort = Integer.parseInt(attributes.getValue("dest-port"));
                BLPacketSender relaySender = new BLPacketSender(destAddr, destPort);
                currentInput.addRelaySender(relaySender);

            } else if (qName.equals("layer")) {
                String inputId = attributes.getValue("input");
                float opacityPct = Float.parseFloat(attributes.getValue("opacity"));
                int x =  Integer.parseInt(attributes.getValue("x"));
                int y =  Integer.parseInt(attributes.getValue("y"));
                int width =  Integer.parseInt(attributes.getValue("width"));
                int height =  Integer.parseInt(attributes.getValue("height"));
                Rectangle layerSize = new Rectangle(x, y, width, height);
                
                Layer l = new Layer(
                        layerSize,
                        AlphaComposite.getInstance(AlphaComposite.SRC_OVER, opacityPct / 100f));
                currentLayer.addLayer(l);
               
                BLPacketReceiver receiver = inputs.get(inputId);
                layerSources.put(l, receiver);
                List<Layer> layersForThisInput = layerInputs.get(receiver);
                if (layersForThisInput == null) {
                    layersForThisInput = new ArrayList<Layer>();
                    BLPacketReceiver input = inputs.get(inputId);
                    if (input == null) {
                        throw new IllegalArgumentException("Couldn't resolve input id: " + inputId);
                    }
                    layerInputs.put(input, layersForThisInput);
                }
                layersForThisInput.add(l);
                currentLayer = l;

            } else if (qName.equals("output")) {
                String destAddr = attributes.getValue("dest-addr");
                int destPort = Integer.parseInt(attributes.getValue("dest-port"));
                long minInterval = Long.parseLong(attributes.getValue("min-frame-interval"));
                PacketType packetFormat = PacketType.valueOf(attributes.getValue("packet-format"));
            	OutputSender outputSender = null; 
                if(packetFormat == PacketType.HACKLAB_SIGN) {
                	outputSender = new HacklabSignDriver(destAddr);
                }
                else {                	
                    outputSender = new BLPacketSender(destAddr, destPort);
                }
                currentOutput = new FixedOutput(outputSender, rootLayer, minInterval, packetFormat);
                outputs.add(currentOutput);

            } else if (qName.equals("screen")) {
                int x =  Integer.parseInt(attributes.getValue("x"));
                int y =  Integer.parseInt(attributes.getValue("y"));
                int width =  Integer.parseInt(attributes.getValue("width"));
                int height =  Integer.parseInt(attributes.getValue("height"));
                int bpp = Integer.parseInt(attributes.getValue("bpp"));
                int screenId = Integer.parseInt(attributes.getValue("screen-id"));

                Rectangle viewport = new Rectangle(x, y, width, height);
                currentOutput.addScreen(viewport, bpp, screenId);
                
            } else if (qName.equals("dynamic-output")) {
                String listenAddr = attributes.getValue("listen-addr");
                int listenPort = Integer.parseInt(attributes.getValue("listen-port"));
                long minInterval = Long.parseLong(attributes.getValue("min-frame-interval"));
                PacketType packetFormat = PacketType.valueOf(attributes.getValue("packet-format"));
                long heartbeatTimeout = Long.parseLong(attributes.getValue("heartbeat-timeout"));
                currentOutput = new DynamicOutput(
                        listenAddr, listenPort, rootLayer,
                        minInterval, packetFormat, heartbeatTimeout);
                ((DynamicOutput) currentOutput).start();
                outputs.add(currentOutput);
            } else if (qName.equals("recording-output")) {
                String directory = attributes.getValue("directory");
                String baseName = attributes.getValue("base-name");
                long minInterval = Long.parseLong(attributes.getValue("min-frame-interval"));
                boolean gzip = "true".equals(attributes.getValue("gzip"));
                currentOutput = new MovieRecorderOutput(rootLayer,new File(directory), baseName, gzip, minInterval);
                outputs.add(currentOutput);
            } else {
	        	BMix.logger.warning("unrecognised entity: " + qName);
	        }

	    } catch (FileFormatException ex) {
	        // avoid wrapping this one in another FileFormatException
	        throw ex;
	    } catch (Exception ex) {
	        throw new FileFormatException(locator.getLineNumber(), locator.getColumnNumber(), ex);
	    }
	}
	
	@Override
	public void endElement(String uri, String localName, String name)
	        throws SAXException {
	    if (name.equals("layer")) {
	        currentLayer = currentLayer.getParentLayer();
	    } else if (name.equals("output")) {
	        if (currentOutput.getViewports().size() < 1) {
	            throw new FileFormatException(
	                    "Output elements must have at least one screen",
	                    locator.getLineNumber(), locator.getColumnNumber());
	        }
	    }
	}
}