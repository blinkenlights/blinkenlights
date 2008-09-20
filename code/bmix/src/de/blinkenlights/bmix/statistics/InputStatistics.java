package de.blinkenlights.bmix.statistics;

import java.awt.Color;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import de.blinkenlights.bmix.network.BLPacketSender;
import de.blinkenlights.bmix.network.BLPacketReceiver.AlphaMode;

public class InputStatistics {
	private final int inputPort;
	private final String heartBeatDestAddr;
	private final int heartBeatDestPort;
	private final AlphaMode alphaMode;
	private final Color chromaKeyColor;
	private final Map<String, Integer> relaySenderMap;
	private final long lastPacketReceiveTime;
	private final String name;

	public InputStatistics(String name, int inputPort, String heartBeatDestAddr, int heartBeatDestPort,
			List<BLPacketSender> relaySenders, AlphaMode alphaMode, Color chromaKeyColor,
		long lastPacketReceiveTime) {
		this.name = name;
		this.inputPort = inputPort;
		this.heartBeatDestAddr = heartBeatDestAddr;
		this.heartBeatDestPort = heartBeatDestPort;
		this.alphaMode = alphaMode;
		this.chromaKeyColor = chromaKeyColor;
		this.lastPacketReceiveTime = lastPacketReceiveTime;
		relaySenderMap = new LinkedHashMap<String, Integer>();
		for(BLPacketSender relaySender: relaySenders) {
			relaySenderMap.put(relaySender.getAddress(), relaySender.getPort());
		}
	}
	
	public String getName() {
		return name;
	}

	public int getInputPort() {
		return inputPort;
	}

	public String getHeartBeatDestAddr() {
		return heartBeatDestAddr;
	}

	public int getHeartBeatDestPort() {
		return heartBeatDestPort;
	}

	public AlphaMode getAlphaMode() {
		return alphaMode;
	}

	public Color getChromaKeyColor() {
		return chromaKeyColor;
	}

	public Map<String, Integer> getRelaySenderMap() {
		return relaySenderMap;
	}

	public long getLastPacketReceiveTime() {
		return lastPacketReceiveTime;
	}
		
	public String toString() {
		StringBuilder str = new StringBuilder();
		str.append("Input - Name: " + name + "\n");
		str.append("  Listen port: " + inputPort + "\n");
		str.append("  Hearbest Dest - Addr: " + heartBeatDestAddr + " - Port: " + heartBeatDestPort + "\n");
		str.append("  Alpha Mode: " + alphaMode.name() + "\n");
		str.append("  Chroma-key Colour: " + chromaKeyColor.toString() + "\n");
		str.append("  Last packet receive time: " + lastPacketReceiveTime + "\n");
		str.append("  Relay Senders: \n");
		for(Map.Entry<String, Integer> ent : relaySenderMap.entrySet()) {
			str.append("    Sender - Addr: " + ent.getKey() + " - Port: " + ent.getValue());
		}
		str.append("\n");
		return str.toString();
	}
}
