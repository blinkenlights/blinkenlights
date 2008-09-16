package de.blinkenlights.bmix.main;

import java.awt.AlphaComposite;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Rectangle;
import java.io.IOException;
import java.io.InputStream;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.xml.sax.Attributes;
import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;
import org.xml.sax.Locator;
import org.xml.sax.SAXException;
import org.xml.sax.XMLReader;
import org.xml.sax.helpers.DefaultHandler;

import de.blinkenlights.bmix.Version;
import de.blinkenlights.bmix.mixer.BLImage;
import de.blinkenlights.bmix.mixer.Layer;
import de.blinkenlights.bmix.mixer.Output;
import de.blinkenlights.bmix.monitor.Monitor;
import de.blinkenlights.bmix.network.BLPacketReceiver;
import de.blinkenlights.bmix.network.BLPacketReceiverThread;
import de.blinkenlights.bmix.network.BLPacketSender;
import de.blinkenlights.bmix.network.BLPacketReceiver.AlphaMode;
import de.blinkenlights.bmix.util.FileFormatException;

/**
 * This is the main class that drives the BMix program. The bmix config filename
 * should be specified on the command line. If it's not, "bmix.xml" will try to
 * be opened instead.
 */
public class BMix extends Monitor {
	
	private static final Logger logger = Logger.getLogger(BMix.class.getName());
	
	private BMixSession session;

    /**
	 * Creates a new BlinkenMix system.
	 * 
	 * @param configFilename the config file to use
	 * @throws SAXException 
	 * @throws ParserConfigurationException 
	 * @throws IOException 
	 */
	public BMix(String configFilename, boolean guiEnabled) throws ParserConfigurationException, SAXException, IOException {
	    super("bmix", 0, 0, 100, 100, guiEnabled);
		BMixSAXHandler saxHandler = new BMixSAXHandler();
		SAXParserFactory spf = SAXParserFactory.newInstance();
		SAXParser sp = null;
		sp = spf.newSAXParser();
        EntityResolver resolver = new EntityResolver() {
            public InputSource resolveEntity(String publicId, String systemId) {
                if (publicId.equals("-//BMix//DTD BMix 1.0//EN")) {
                    InputStream in = BMix.class.getResourceAsStream(
                            "/de/blinkenlights/bmix/bmix.dtd");
                    return new InputSource(in);
                }
                return null;
            }
        };
        XMLReader reader = sp.getXMLReader();
        reader.setEntityResolver(resolver);
        reader.setContentHandler(saxHandler);
        reader.parse(new InputSource(configFilename));	
		session = saxHandler.getConfiguration();
	}
	
	/**
	 * This is the official, government-approved method to invoke bmix.
	 * </p><p>
	 * Valid Command-Line Arguments:
	 * <dl>
	 *  <dt><marquee>configFilename</marquee></dt><dd>the name of the config file to use</dd>
	 * </dl>
	 * @param args command line arguments
	 */
	public static void main(String[] args) throws Exception {
		int verbosity = 0;
		boolean gotFilename = false;
		boolean guiEnabled = true;
		
		String configFilename = "bmix.xml";

		for (String arg : args) {
			if (!arg.startsWith("-")) {
				if (!gotFilename) {
					configFilename = arg;
					gotFilename = true;
				} else {
					showHelp();
					return;
				}
			}
			else if ("-v".equals(arg)) {
				verbosity++;
			}
			else if ("-d".equals(arg)) {
				guiEnabled = false;
			} else {
				showHelp();
				return;
			}
			
			if (verbosity >= 50) {
				System.out.println("wow, we're getting really verbose now...");
			}
			
		}
		
		Level logLevel;
		if (verbosity == 0) {
			logLevel = Level.WARNING;
		}
		else if (verbosity == 1) {
			logLevel = Level.INFO;
		}
		else if (verbosity == 2) {
			logLevel = Level.FINE;
		}
		else if (verbosity >= 3) {
			logLevel = Level.FINEST;
		} else {
			System.err.println("Fatal Error: Invalid log verbosity: " + verbosity);
			return;
		}
		System.err.println("Setting log level to " + logLevel);
		Logger.getLogger("").setLevel(logLevel);
		for (Handler handler : Logger.getLogger("").getHandlers()) {
		    handler.setLevel(logLevel);
		}

		new BMix(configFilename, guiEnabled).start();
	}
	
	private static void showHelp() {
	    System.out.println("BMix version " + Version.VERSION);
	    System.out.println(" Usage: bmix [-v] [-d] [config filename]");
	    System.out.println("  -v increase verbosity (use more for more verbosity)");
	    System.out.println("  -d disable gui");
	}

	private static class BMixSAXHandler extends DefaultHandler {
		
		/**
		 * Document locator provided by SAX parser. Helps when reporting errors.
		 */
        private Locator locator;

        /**
         * Maps input IDs to the packet receivers responsible for those inputs.
         */
        private final Map<String, BLPacketReceiver> inputs = new HashMap<String, BLPacketReceiver>();
        
        /**
         * Mapping of inputs to the layers they feed.
         */
        private final Map<BLPacketReceiver, List<Layer>> layerInputs = new HashMap<BLPacketReceiver, List<Layer>>();

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
		        session = new BMixSession(rootLayer, layerInputs, outputs, maxFrameInterval);
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
		            InetAddress heartBeatDestAddr = null;
		            if (heartBeatDestAddrString != null) {
		            	heartBeatDestAddr = InetAddress.getByName(heartBeatDestAddrString);
		            } 
		            int heartBeatDestPort = Integer.parseInt(attributes.getValue("heartbeat-dest-port"));
		            BLPacketReceiver receiver = 
		                new BLPacketReceiver(
		                        Integer.parseInt(listenPort),
		                        InetAddress.getByName(listenAddr),
		                        heartBeatDestAddr, heartBeatDestPort, alphaMode, transparentColour);
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
                    int x =  Integer.parseInt(attributes.getValue("x"));
                    int y =  Integer.parseInt(attributes.getValue("y"));
                    int width =  Integer.parseInt(attributes.getValue("width"));
                    int height =  Integer.parseInt(attributes.getValue("height"));
                    long minInterval = Long.parseLong(attributes.getValue("min-frame-interval"));
                    
                    Rectangle viewport = new Rectangle(x, y, width, height);

                    BLPacketSender sender = new BLPacketSender(destAddr, destPort);
                    Output output = new Output(sender, rootLayer, viewport, minInterval);
                    outputs.add(output);
                    
		        } else {
		        	logger.warning("unrecognised entity: " + qName);
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
		    }
		}
	}

    @Override
    protected BLImage getNextImage() {
        logger.entering("BMix", "getNextImage");
        
        session.waitForNewPacket();
        
        for (BLPacketReceiverThread t : session.getReceiverThreads()) {
            t.updateTargetLayers(session.getLayersForReceiver(t.getReceiver()));
        }
        
        session.getRootLayer().mixdown();
        
        for (Output output : session.getOutputs()) {
            try {
                output.send();
            } catch (Exception e) {
                logger.log(Level.WARNING, "Failed to send on output " + output,e);
            }
        }
        
        logger.exiting("BMix", "getNextImage", session.getRootLayer());
        return session.getRootLayer();
    }
	
}
