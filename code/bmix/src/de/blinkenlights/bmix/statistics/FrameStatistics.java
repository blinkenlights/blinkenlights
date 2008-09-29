package de.blinkenlights.bmix.statistics;

import java.io.Serializable;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import de.blinkenlights.bmix.mixer.BLImageViewport;
import de.blinkenlights.bmix.mixer.Layer;
import de.blinkenlights.bmix.mixer.Output;
import de.blinkenlights.bmix.network.BLPacketReceiver;

public class FrameStatistics implements Serializable {
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
	
		// go through all the inputs
		for (BLPacketReceiver input : layerInputs.keySet()) {
			InputStatistics inputStat = new InputStatistics(input);
			inputStats.add(inputStat);
			inputReceiverMap.put(input, inputStat);
		}
	}

	
	private void addLayers(Layer layer, Map<Layer, BLPacketReceiver> layerSources) {
		BLPacketReceiver receiver = layerSources.get(layer);
		InputStatistics inputStat = inputReceiverMap.get(receiver); // note: can be null (eg, root layer)
		LayerStatistics layerStat = new LayerStatistics(inputStat, layer);
		layerStats.add(0, layerStat);
		
		for (Layer l : layer.getLayers()) {
			addLayers(l, layerSources);
		}
	}

	
	private void addOutputs(List<Output> outputs) {
		for(Output output : outputs) {
		    // TODO multiple subframes
		    BLImageViewport subframe0 = output.getViewports().get(0);
			OutputStatistics outputStat = new OutputStatistics(
					System.identityHashCode(output),
					subframe0.getViewport(), 
					output.getDestinations(), output.getMinSendInterval(), 
					output.getPacketType(), subframe0.getBpp());
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
	
	public List<InputStatistics> getInputStats() {
		return inputStats;
	}
	
	public LinkedList<LayerStatistics> getLayerStats() {
		return layerStats;
	}
	
	public List<OutputStatistics> getOutputStats() {
		return outputStats;
	}
}
