/* 
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

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import de.blinkenlights.bmix.mixer.Layer;
import de.blinkenlights.bmix.mixer.Output;
import de.blinkenlights.bmix.network.BLPacketReceiver;
import de.blinkenlights.bmix.network.BLPacketReceiverThread;

/**
 * This class represents a BMixSession.
 */
public final class BMixSession {
    private final Layer rootLayer;
    private final Map<BLPacketReceiver, List<Layer>> layerInputs;
    
    /**
     * An unmodifiable copy of the list given in the constructor.
     */
    private final List<BLPacketReceiverThread> receiverThreads;

    private final Semaphore semaphore = new Semaphore(0);
    private final long maxFrameInterval;
    
    /**
     * An unmodifiable copy of the list given in the constructor.
     */
    private final List<Output> outputs;
    
	private final Map<Layer, BLPacketReceiver> layerSources;
    
    
    /**
     * Sets up this configuration, creating the necessary receiver threads and
     * starting them.
     * 
     * @param rootLayer
     *            The special root layer for this configuration. It should not
     *            have a parent, and all other layers must be descendants of
     *            this layer.
     * @param outputs
     *            The outputs this session sends new frame data to.
     * @param layerInputs
     *            A map of each packet receiver to all the layers they feed data
     *            to.
     * @param layerSources 
     * @param outputs
     *            The outputs for this session
     */
    public BMixSession(Layer rootLayer, Map<BLPacketReceiver, List<Layer>> layerInputs, Map<Layer, BLPacketReceiver> layerSources, 
    		List<Output> outputs, long maxFrameInterval) {
        this.rootLayer = rootLayer;
        this.layerInputs = layerInputs;
		this.layerSources = layerSources;
        this.outputs = Collections.unmodifiableList(new ArrayList<Output>(outputs));
        this.maxFrameInterval = maxFrameInterval;
        
        List<BLPacketReceiverThread> threads = new ArrayList<BLPacketReceiverThread>();
        for (BLPacketReceiver r : layerInputs.keySet()) {
            BLPacketReceiverThread t = new BLPacketReceiverThread(r, semaphore, r.getTimeoutMillis());
            t.start();
            threads.add(t);
        }
        receiverThreads = Collections.unmodifiableList(threads);

    }

    
    public void close() {
        for (BLPacketReceiver layerInput : layerInputs.keySet()) {
        	layerInput.close();
        }
        
        for (BLPacketReceiverThread receiverThread : receiverThreads) {
        	receiverThread.close();
        }
        
        for (Output output : outputs)  {
        	output.close();
        }
    }
    
    
    public Layer getRootLayer() {
        return rootLayer;
    }
    
    public List<Layer> getLayersForReceiver(BLPacketReceiver r) {
        List<Layer> layers = layerInputs.get(r);
        if (layers == null) {
            return Collections.emptyList();
        } else {
            return layers;
        }
    }
    
    
    /**
     * Gets the inputs mapped to layers.
     * 
     * @return the inputs mapped to layers
     */
    public Map<BLPacketReceiver, List<Layer>> getLayerInputs() {
		return layerInputs;
	}
    
    
    /**
     * Gets the layers mapped to source inputs.
     * 
     * @return the layers mapped to source inputs.
     */
    public Map<Layer, BLPacketReceiver> getLayerSources() {
    	return layerSources;
    }
    

	/**
     * Returns the list of receiver threads for this configuration. There will
     * be one thread per receiver.
     * 
     * @return A list you can't modify, so don't try.
     */
    public List<BLPacketReceiverThread> getReceiverThreads() {
        return receiverThreads;
    }
    
     
    /**
     * Returns the list of outputs for this configuration.
     * 
     * @return A list you can't modify, so don't try.
     */
    public List<Output> getOutputs() {
        return outputs;
    }

    /**
     * Waits either for one of the receiver threads to get a new packet
     * from its packet receiver, or until your thread is interrupted, whichever
     * comes first.
     */
    public void waitForNewPacket() {
        try {
            semaphore.tryAcquire(maxFrameInterval, TimeUnit.MILLISECONDS);
        } catch (InterruptedException e) {
            // it's ok to pretend there was new data
        }
    }

}
