package de.blinkenlights.bmix.movie;

import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.LinkedList;

import javax.security.sasl.SaslException;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.xml.sax.Attributes;
import org.xml.sax.InputSource;
import org.xml.sax.Locator;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import de.blinkenlights.bmix.util.FileFormatException;

/**
 * This class implements a quick and dirty movie reader class for parsing a BML or BLM movie file and
 * loading each frame. Frames can then be requested from the movie. Not all quirks of the various
 * BL movie formats are supported.
 * 
 * The movie format is determined by the filename extension. .blm = legacy format, .bml = XML format
 * 
 * Currently only 1 and 4 bit movies are supported. If a movie file is missing the bits attribute
 * in the blm entity, a 1 bit movie is assumed. (this appears to be the case for older 1 bit movies)
 */
public class BLMovie {
	LinkedList<Frame> frames;

	/**
	 * Creates a BMovie from a BML XML file.
	 * 
	 * @param filename the BML movie to load
	 */
	public BLMovie(String filename) {
		frames = new LinkedList<Frame>();
		
		if(filename.length() < 5) {
			System.out.println("BLMovie filename is invalid: " + filename);
			return;
		}
		String suffix = filename.substring(filename.length() - 3, filename.length());
		if(suffix.toUpperCase().equals("BLM")) {
			parseLegacyMovie(filename);
		}
		else if(suffix.toUpperCase().equals("BML")) {
			ElementParser ep = new ElementParser(frames);
			SAXParserFactory spf = SAXParserFactory.newInstance();
			SAXParser sp = null;
			try {
				sp = spf.newSAXParser();
				sp.parse(new File(filename), ep);	
			} catch (ParserConfigurationException e) {
				e.printStackTrace();
				System.exit(1);
			} catch (SAXException e) {
				e.printStackTrace();
				System.exit(1);
			} catch (IOException e) {
				e.printStackTrace();
				System.exit(1);
			}			
		}
		else {
			System.err.println("file format not supported: " + suffix);
		}
	}
	
	/**
	 * Gets the number of frames.
	 * 
	 * @return the number of frames
	 */
	public int getNumFrames() {
		return frames.size();
	}
	
	/**
	 * Gets a frame.
	 */
	public Frame getFrame(int index) {
		return frames.get(index);
	}
	
	/**
	 * Parses a legacy movie file. (BLM)
	 * 
	 * @param filename the filename to parse
	 */
	private void parseLegacyMovie(String filename) {
		BufferedReader in = null;
		try {
			in = new BufferedReader(new FileReader(filename));
		} catch (FileNotFoundException e) {
			e.printStackTrace();
			return;
		}
		
		String line;
		try {
			int duration = 0;
			LinkedList<String> rows = new LinkedList<String>();;
			while((line = in.readLine()) != null) {
				line = line.trim();
				// line contains data
				if(line.length() > 1) {
					// ignore comments
					if(line.charAt(0) == '#') {
						// do nothing
					}
					// get a frame duraction
					else if(line.charAt(0) == '@') {
						try {
							duration = Integer.parseInt(line.substring(1, line.length()));
							if(duration < 1) throw new NumberFormatException("duration must be > 0");
							System.out.println("duration: " + duration);
						} catch(NumberFormatException e) {
							throw new IOException("error parsing duration for frame: " + e.getMessage());
						}
					}
					// get pixels
					else {
						rows.addLast(line);
					}
				}
				// line is blank
				else {
					// if we have rows let's make a frame from it
					if(rows.size() > 0) {
						System.out.println("got some rows: " + rows.size());
						if(duration < 1) {
							throw new IOException("duration not set for frame!");
						}
						int width = rows.get(0).length();
						int height = rows.size();
						Frame frame = new Frame(width, height, 1, duration);
						for(int i = 0; i < rows.size(); i ++) {
							frame.addRow(rows.get(i));
						}
						frames.addLast(frame);
						rows.clear();
					}
					else {
						System.out.println("there aren't any rows");
					}
				}
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	/**
	 * Class for parsing the library file.
	 */
	static class ElementParser extends DefaultHandler {
		Locator locator;
		LinkedList<Frame> frames;
		Frame frame = null;
		int width = 0;
		int height = 0;
		int bits = 0;
		int level = 0;
		StringBuffer cdata;
		
		/**
		 * Creates a new ElementParser for parsing the document.
		 * 
		 * @param library the image library to use
		 */
		public ElementParser(LinkedList<Frame> frames) {
			this.frames = frames;
			cdata = new StringBuffer();
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
		
		@Override
		public void setDocumentLocator(Locator locator) {
			this.locator = locator;
		}
		
		/**
		 * Handles a new element.
		 * 
		 * @param uri namespace URI
		 * @param localName the local name (without prefix)
		 * @param qName the qualified name (with prefix)
		 * @param attributes the attributes for this element 
		 * @throws SaslException 
		 */
		public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
			cdata = new StringBuffer();
			// blm
			if(qName.equals("blm")) {
				level ++;
				// width
				String val = attributes.getValue("width");
				if(val == null) {
					throw new FileFormatException("width not specified", locator.getLineNumber(), locator.getColumnNumber());
				}
				width = Integer.parseInt(val);
				// height
				val = attributes.getValue("height");
				if(val == null) {
					throw new FileFormatException("height not specified", locator.getLineNumber(), locator.getColumnNumber());
				}
				height = Integer.parseInt(val);
				// bits
				val = attributes.getValue("bits");
				if(val == null) {
					// if bits is not specified, consider that it's 1
					bits = 1;
				}
				else {
					bits = Integer.parseInt(val);
				}
				if(bits != 4 && bits != 3 && bits != 2 && bits != 1) {
					throw new FileFormatException("bits must be 1, 2, 3 or 4", locator.getLineNumber(), locator.getColumnNumber());
				}
			}
			else if(qName.equals("frame") && level == 1) {
				level ++;
				String val = attributes.getValue("duration");
				String val2 = attributes.getValue("ms");
				if(val == null && val2 == null) {
					throw new FileFormatException("duration not specified", locator.getLineNumber(), locator.getColumnNumber());
				}
				if(val2 != null) {
					val = val2;
				}
				int duration = Integer.parseInt(val);
				frame = new Frame(width, height, bits, duration);
				frames.addLast(frame);
				level ++;
			}
			else if(qName.equals("row") && level == 2) {
				level ++;
			}
			else {
//				System.err.println("unrecognised entity: " + qName + " at level: " + level);
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
			if(qName.equals("blm")) {
				// ignore
			}
			// frame
			if(qName.equals("frame") && level == 2) {
				level --;
			}
			// row
			else if(qName.equals("row") && level == 3) {
				frame.addRow(cdata.toString());
				level --;
			}
		}

		/**
		 * Handles character data.
		 */
		public void characters(char ch[], int start, int len) {
			cdata.append(ch, start, len);
		}
	}
}
