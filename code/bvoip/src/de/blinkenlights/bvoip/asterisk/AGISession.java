package de.blinkenlights.bvoip.asterisk;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Map;
import java.util.TreeMap;
import java.util.concurrent.ConcurrentLinkedQueue;
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

	private final Map<String, String> headers;
	
	/**
	 * When other threads want to do telephony things, they will simply
	 * add a command to this queue, and the thread responsible for this
	 * AGI session will actually consume the command some time later. This
	 * ensures any CallEndedException will be thrown on the correct thread. 
	 */
	private final ConcurrentLinkedQueue<String> agiCommandQueue = new ConcurrentLinkedQueue<String>();
	
	/**
	 * This is the list of digits we've detected from the phone call and
	 * have not been consumed by a client yet.
	 */
	private final ConcurrentLinkedQueue<Character> digitQueue = new ConcurrentLinkedQueue<Character>();
	
	AGISession(BufferedReader in, PrintWriter out) throws IOException {
		this.in = in;
		this.out = out;
		headers = new TreeMap<String, String>();
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
	}
	
	public void handleCall() throws IOException, CallEndedException {
		logger.entering("AGISession", "handleCall");
		
		StringBuilder digitLog = new StringBuilder();
		for (;;) {
			String command;
			if ( (command = agiCommandQueue.poll()) != null) {
				serverCommand(command); // TODO capture the return value into a Future
			}
			String response = serverCommand("WAIT FOR DIGIT 1000");
			char digit = (char) Integer.parseInt(response);
			if (digit > 0) {
				logger.fine("Digit: " + response);
				digitLog.append(digit);
				digitQueue.offer(digit);
				logger.fine("All digits this call: " + digitLog);
			}
		}
	}
	
	public String getDnid() {
		return headers.get(AGI_DNID);
	}
	
	public String getCallerId() {
		return headers.get(AGI_CALLERID);
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

	/**
	 * Answers the call.
	 */
	public void answer() {
		agiCommandQueue.add("ANSWER");
	}

	/**
	 * Hangs up the call.
	 */
	public void hangup() {
		agiCommandQueue.add("HANGUP");
	}
	
	/**
	 * Plays a background music context.
	 * 
	 * @param mohContext the context to play
	 */
	public void playBackground(String mohContext) {
		agiCommandQueue.add("SET MUSIC on " + mohContext);
	}
	
	/**
	 * Returns the next unconsumed digit that was received from Asterisk.
	 * If there are no digits in the queueue, returnrns nulull.
	 * 
	 * @return One of the unicode characters that can be transmitted using DTMF.
	 */
	public Character getDigit() {
		return digitQueue.poll();
	}
}
