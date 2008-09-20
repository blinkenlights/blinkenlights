package de.blinkenlights.bmix.statistics;

import java.awt.Color;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import de.blinkenlights.bmix.mixer.Layer;
import de.blinkenlights.bmix.mixer.Output;
import de.blinkenlights.bmix.network.BLPacketReceiver;
import de.blinkenlights.bmix.network.BLPacketSender;
import de.blinkenlights.bmix.network.BLPacketReceiver.AlphaMode;

public class FrameStatistics implements StatisticsItem {
	private static final long serialVersionUID = 1234567890L;
	
	transient Map<BLPacketReceiver, InputStatistics> inputReceiverMap;
	List<InputStatistics> inputStats;
	LinkedList<LayerStatistics> layerStats;
	List<OutputStatistics> outputStats; 

	
	public FrameStatistics(Map<BLPacketReceiver, List<Layer>> layerInputs, Layer rootLayer, 
			Map<Layer, BLPacketReceiver> layerSources, List<Output> outputs) {
		inputStats = new LinkedList<InputStatistics>();
		layerStats = new LinkedList<LayerStatistics>();
		outputStats = new LinkedList<OutputStatistics>();
		
		addInputs(layerInputs);
		addLayers(rootLayer, layerSources);
		addOutputs(outputs);
	}

	private void addInputs(Map<BLPacketReceiver, List<Layer>> layerInputs) {
		inputReceiverMap = new HashMap<BLPacketReceiver, InputStatistics>();
		Iterator<BLPacketReceiver> inputs = layerInputs.keySet().iterator();
	
		// go through all the inputs
		while(inputs.hasNext()) {
			BLPacketReceiver input = inputs.next();
			int inputPort = input.getPort();
			String heartBeatDestAddr = input.getHeartBeatDestAddr();
			int hearBeatDestPort = input.getHeartBeatDestPort();
			List<BLPacketSender> relaySenders = input.getRelaySenders();
			AlphaMode alphaMode = input.getAlphaMode();
			Color chromaKeyColor = input.getTransparentColour();
			long lastPacketReceiveTime = input.getLastPacketReceiveTime();
			InputStatistics inputStat = new InputStatistics(input.getName(), inputPort, 
					heartBeatDestAddr, hearBeatDestPort,
					relaySenders, alphaMode, chromaKeyColor, lastPacketReceiveTime);
			inputStats.add(inputStat);
			inputReceiverMap.put(input, inputStat);
		}
	}

	
	private void addLayers(Layer layer, Map<Layer, BLPacketReceiver> layerSources) {
		BLPacketReceiver receiver = layerSources.get(layer);
		InputStatistics inputStat = inputReceiverMap.get(receiver); // note: can be null (eg, root layer)
		LayerStatistics layerStat = new LayerStatistics(inputStat, layer.getViewport(), layer.getOpacity());
		layerStats.add(0, layerStat);
		
		for (Layer l : layer.getLayers()) {
			addLayers(l, layerSources);
		}
	}

	
	private void addOutputs(List<Output> outputs) {
		for(Output output : outputs) {
			OutputStatistics outputStat = new OutputStatistics(output.getViewport().getViewport(), 
					output.getDestAddr(), output.getDestPort(), output.getMinSendInterval(), 
					output.getPacketType().name(), output.getMultiframeBpp());
			outputStats.add(outputStat);
		}
	}
	
	
	public String toString() {
		StringBuilder str = new StringBuilder();
		str.append("Input Statistics:\n");
		for(InputStatistics inputStat : inputStats) {
			str.append("  " + inputStat.toString());
		}
		str.append("\nLayer Statistics:\n");
		for(LayerStatistics layerStat : layerStats) {
			str.append("  " + layerStat.toString());
		}
		str.append("\nOutput Statistics:\n");
		for(OutputStatistics outputStat : outputStats) {
			str.append("  " + outputStat.toString());
		}
		return str.toString();
	}
	
	public String toHtml() {
		return "<html><p>Overall frame stats";
	}

	public List<StatisticsItem> getChildren() {
		List<StatisticsItem> children = new ArrayList<StatisticsItem>();
		children.addAll(inputStats);
		children.addAll(layerStats);
		children.addAll(outputStats);
		return children;
	}

	public String getName() {
		return "Overall frame statistics";
	}
}
