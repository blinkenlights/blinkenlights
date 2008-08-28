/*
 * Created on Aug 27, 2008
 *
 * This code belongs to yo' mama
 */
package de.blinkenlights.bmix.monitor;

import java.util.LinkedList;
import java.util.Queue;

public class NSecondAverage implements NumberMuncher {

    private static class Entry {

        long time;
        float value;
        
        public Entry(long time, float value) {
            super();
            this.time = time;
            this.value = value;
        }
    }
    
    Queue<Entry> entries = new LinkedList<Entry>();
    
    private float currentValue;
    
    private final long window;
    
    NSecondAverage(long windowMS) {
        this.window = windowMS;
        
    }
    
    public float getCurrentValue() {
        return currentValue;
    }

    public float putValue(float v) {
        long now = System.currentTimeMillis();
        while (entries.size() > 0 && entries.peek().time > now - window) {
            entries.remove();
        }
        entries.add(new Entry(now, v));
        
        float newValue = 0f;
        for (Entry e : entries) {
            newValue += e.value;
        }
        currentValue = newValue / entries.size();
        return currentValue;
    }

    public void flush() {
        entries.clear();
    }

}
