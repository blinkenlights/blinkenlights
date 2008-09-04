package de.blinkenlights.bvoip.asterisk;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Map;
import java.util.TreeMap;
import java.util.logging.Logger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class AGISession {
	private static final Logger logger = Logger.getLogger(AGISession.class.getName());

	/**
	 * The originating network's caller id. Usually the caller's phone number.
	 */
	public static final String AGI_CALLERID = "agi_callerid";
	
	/**
	 * The originating network's textual caller id. Usually the subscriber's name.
	 * An endless source of shits and giggles.
	 */
	public static final String AGI_CALLERIDNAME = "agi_calleridname";
	
	/**
	 * The receiving (asterisk) phone number that was called.
	 */
	public static final String AGI_DNID = "agi_dnid";
	
	private final long startTime = System.currentTimeMillis();
	
	private final PrintWriter out;
	private final BufferedReader in; 
	
	AGISession(BufferedReader in, PrintWriter out) {
		this.in = in;
		this.out = out;
	}
	
	public void handleCall() throws IOException, CallEndedException {
		logger.entering("AGISession", "handleCall");
		Map<String, String> headers = new TreeMap<String, String>();
		Pattern agiHeader = Pattern.compile("^([^:]*): (.*)$");
		String line;
		while((line = in.readLine()).length() > 0) {
			Matcher m = agiHeader.matcher(line);
			if (m.matches()) {
				headers.put(m.group(1), m.group(2));
			} else {
				logger.severe("Unexpected AGI header format from asterisk: \""+line+"\"");
			}
		}
		
		logger.fine("AGI Headers: " + headers);
		
		serverCommand("ANSWER");
		
		StringBuilder digitLog = new StringBuilder();
		for (;;) {
			String response = serverCommand("WAIT FOR DIGIT 1000");
			logger.fine("Digit: " + response);
			digitLog.append((char) Integer.parseInt(response));
			logger.fine("All digits this call: " + digitLog);
		}
	}
	
	private String serverCommand(String command) throws CallEndedException, IOException {
		long elapsed = System.currentTimeMillis() - startTime;
		logger.finest(elapsed + ": Sending server command \""+command+"\"");
		out.println(command);
		String response = in.readLine();
		if (response == null) throw new CallEndedException();
		Pattern p = Pattern.compile("200 result=([^ ]*)");
		Matcher m = p.matcher(response);
		if (m.matches()) {
			return m.group(1);
		} else {
			throw new UnsupportedOperationException("Unexpected server response to command " + command + ": " + response);
		}
	}
}
