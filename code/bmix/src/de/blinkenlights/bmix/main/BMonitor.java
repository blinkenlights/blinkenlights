package de.blinkenlights.bmix.main;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.IOException;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.xml.sax.Attributes;
import org.xml.sax.InputSource;
import org.xml.sax.Locator;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import de.blinkenlights.bmix.monitor.NetworkStreamMonitor;
import de.blinkenlights.bmix.monitor.NetworkStreamMonitorConfig;
import de.blinkenlights.bmix.util.FileFormatException;

/**
 * This class contains the main for the multi-stream BMonitor program.
 * This program can listen to many BL frame streams at once and display them
 * in separate windows on the screen. All ports and windows are configured
 * in a single XML file. 
 */
public class BMonitor {
	
	private final static Logger logger = Logger.getLogger(BMonitor.class.getName());
	
	/**
	 * Creates a new BMonitor.
	 * 
	 * @param configFilename
	 * @throws SAXException If the config file can't be parsed.
	 * @throws ParserConfigurationException If the config file parser can't be created.
	 * @throws IOException If the config file can't be read
	 */
	public BMonitor(String configFilename) throws ParserConfigurationException, SAXException, IOException {	
		LinkedList<NetworkStreamMonitorConfig> monitorList = new LinkedList<NetworkStreamMonitorConfig>();
		ElementParser ep = new ElementParser(monitorList);
		SAXParserFactory spf = SAXParserFactory.newInstance();
		SAXParser sp = null;
		sp = spf.newSAXParser();
		sp.parse(new File(configFilename), ep);	
		
		Iterator<NetworkStreamMonitorConfig> iter = monitorList.iterator();
		while(iter.hasNext()) {
			NetworkStreamMonitorConfig nc = iter.next();
			NetworkStreamMonitor nsm = new NetworkStreamMonitor(nc);
			logger.info("BMonitor - setting up monitor - name: " + nc.getName() + " - port: " + nc.getPort());
			nsm.start();
		}
	}
	
	/**
	 * Main!
	 * 
	 * @param args
	 */
	public static void main(String[] args) throws Exception {
		if(args.length < 1) {
			System.err.println("BMonitor - config filename must be given");
			return;
		}
		new BMonitor(args[0]);
	}		
	
	/**
	 * Class for parsing the BMonitor config.
	 */
	static class ElementParser extends DefaultHandler {
		LinkedList<NetworkStreamMonitorConfig> monitors;
		Locator locator;

		@Override
		public void setDocumentLocator(Locator locator) {
			this.locator = locator;
		}

		/**
		 * Creates a new ElementParser for parsing the document.
		 * 
		 * @param monitorList the list of monitors to populate
		 */
		public ElementParser(LinkedList<NetworkStreamMonitorConfig> monitorList) {
			this.monitors = monitorList;
		}

		/**
		 * Override the doctype so that the parser will work offline.
		 */
		public InputSource resolveEntity(String publicId, String systemId) throws SAXException, IOException {
			return new InputSource(new ByteArrayInputStream("<?xml version='1.0' encoding='UTF-8'?>".getBytes()));
		}
		
		/**
		 * Handles the start of a document.
		 */
		public void startDocument() {
			// ignore
		}
		
		/**
		 * Handles the end of the a document.
		 */
		public void endDocument() {
			// ignore
		}
		
		/**
		 * Handles a new element.
		 * 
		 * @param uri namespace URI
		 * @param localName the local name (without prefix)
		 * @param qName the qualified name (with prefix)
		 * @param attributes the attributes for this element 
		 * @throws FileFormatException on parse error 
		 */
		public void startElement(String uri, String localName, String qName, Attributes attributes) throws FileFormatException {
			if(qName.equals("bmonitor")) {
				// ignore
			}
			else if(qName.equals("monitor")) {
				String val = attributes.getValue("port");
				if(val == null) {
					throw new FileFormatException("port must be specified", locator.getLineNumber(), locator.getColumnNumber());
				}
				int port = 0;
				int x = 0;
				int y = 0;
				int w = 200;
				int h = 200;
				try {
					port = Integer.parseInt(val);
					val = attributes.getValue("x");
					if(val != null) x = Integer.parseInt(val);
					val = attributes.getValue("y");
					if(val != null) y = Integer.parseInt(val);
					val = attributes.getValue("width");
					if(val != null) w = Integer.parseInt(val);
					val = attributes.getValue("height");
					if(val != null) h = Integer.parseInt(val);
				} catch(NumberFormatException e) {
					throw new FileFormatException("expected a number, got something that wasn't a number",locator.getLineNumber(), locator.getColumnNumber());
				}
				String temp = "BMonitor - ";
				String name = attributes.getValue("name");
				if(name != null) temp += name;		
				temp += " - Port: " + Integer.toString(port);
			    monitors.add(new NetworkStreamMonitorConfig(port, temp, x, y, w, h));
			}
			else {
				logger.warning("unrecognised entity: " + qName);
			}
		}
		
		/**
		 * Handles the end of an element.
		 * 
		 * @param uri namespace URI
		 * @param localName the local name (without prefix)
		 * @param qName the qualified name (with prefix)
		 */
		public void endElement(String uri, String localName, String qName) {
			// ignore
		}
		
		/**
		 * Handles character data.
		 */
		public void characters(char ch[], int start, int len) {
			// ignore
		}
	}	
}
