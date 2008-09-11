package de.blinkenlights.bvoip.blt;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
import java.util.logging.Logger;

public class BLTCommand {
	private static final Logger logger = Logger.getLogger(BLTCommand.class.getName());

	public enum CommandType {
		ONHOOK,
		SETUP,
		CONNECTED,
		DTMF,
		ERROR,
		REGISTER,
		HEARTBEAT,
		ACCEPT,
		HANGUP,
		PLAYBACKGROUND,
		PLAY
	};

	private final CommandType command;
	private final int channelNum;
	
	private final List<String> args;
	
	public BLTCommand(int channelNum, CommandType command) {
		this.args = new LinkedList<String>();
		this.channelNum = channelNum;
		this.command = command;
	}
	
	public BLTCommand(int channelNum, CommandType command, String ... args) {
		this.args = new ArrayList<String>(Arrays.asList(args));
		this.channelNum = channelNum;
		this.command = command;
	}
	
	public static BLTCommand parsePacket(String data) {
		int channel; 
		String command;

		String parts[] = data.trim().split(":");
		if (parts.length < 2) {
			throw new IllegalArgumentException("not enough parts in message ("+data+")");
		}
		try {
			channel = Integer.parseInt(parts[0]);
		} catch (NumberFormatException e) { 
			throw new IllegalArgumentException("first part of message must be a number in message ("+data+")");			
		}
		command = parts[1];
		BLTCommand retval = null;
		if ("register".equals(command)) {
			logger.fine("register command");
			retval = new BLTCommand(channel, CommandType.REGISTER);
			if (parts.length > 2) {
				// optional destination port
				retval.args.add(parts[2]);
			}
		}
		else if ("heartbeat".equals(command)) {
			logger.fine("heartbeat command");
			retval = new BLTCommand(channel, CommandType.HEARTBEAT);
		}
		else if ("accept".equals(command)) {
			logger.fine("accept command");
			retval = new BLTCommand(channel, CommandType.ACCEPT);
		}
		else if ("hangup".equals(command)) {
			logger.fine("hangup command");
			retval = new BLTCommand(channel, CommandType.HANGUP);
			if (parts.length > 2) {
				// optional reason
				retval.args.add(parts[2]);
			}

		}
		else if ("playbackground".equals(command)) {
			logger.fine("playbackground command");
			retval = new BLTCommand(channel, CommandType.PLAYBACKGROUND);
			if (parts.length > 2) {
				// filename to play
				retval.args.add(parts[2]);
			} else {
				throw new IllegalArgumentException("playbackground requires one argument, got 0.");
			}
		}
		else if ("play".equals(command)) {
			logger.fine("play command");		
			retval = new BLTCommand(channel, CommandType.PLAY);
			if (parts.length > 2) {
				// filename to play
				retval.args.add(parts[2]);
			} else {
				throw new IllegalArgumentException("play requires one argument, got 0.");
			}
		}
		else {
			logger.warning("unknown command received: " + command);
		}	
		logger.finest("returning "+retval.command+" Command with arguments: "+retval.args);
		return retval;
	}

	public int getChannelNum() {
		return channelNum;
	}
	
	public CommandType getCommand() {
		return command;
	}
	
	public List<String> getArgs() {
		return Collections.unmodifiableList(args);
	}

	public byte[] getNetworkBytes() {
		int channelNum = this.channelNum+1;
		String output = "";
		if (command == CommandType.ONHOOK) {
			output = channelNum+":"+"onhook\n";
		}
		else if (command == CommandType.SETUP) {
			if (args.size() != 2) {
				throw new IllegalArgumentException("setup requires 2 arguments, you provided "+args.size());
			}
			output = channelNum+":"+"setup:"+args.get(0)+":"+args.get(1)+"\n";
		}
		else if (command == CommandType.CONNECTED) {
			output = channelNum+":"+"connected\n";
		}
		else if (command == CommandType.DTMF) {
			if (args.size() != 1) {
				throw new IllegalArgumentException("dtmf requires 1 argument, you provided "+args.size());
			}
			output = channelNum+":"+"dtmf:"+args.get(0)+"\n";
		}
		else if (command == CommandType.ERROR) {
			if (args.size() != 1) {
				throw new IllegalArgumentException("error requires 1 argument, you provided "+args.size());
			}
			output = channelNum+":"+"error:"+args.get(0)+"\n";
		}
		return output.getBytes();
	}
}
