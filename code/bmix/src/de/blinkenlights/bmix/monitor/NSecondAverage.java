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
