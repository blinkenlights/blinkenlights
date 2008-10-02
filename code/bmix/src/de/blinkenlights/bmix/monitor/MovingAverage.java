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

/**
 * MegaMovingAverageIsAClassForWhichRichardStallmanCantReadTheDescription.
 */
public class MovingAverage implements NumberMuncher {

    private final float samples[];
    private float currentAverage;
    private int cursor;
    boolean firstTimeAround;
    
    MovingAverage(int nsamples) {
        samples = new float[nsamples];
        flush();
    }

    public void flush() {
        cursor = 0;
        firstTimeAround = true;
    }
    
    public float putValue(float v) {
        samples[cursor++] = v;
        if (cursor >= samples.length) {
            cursor = 0;
            firstTimeAround = false;
        }
        currentAverage = calcAverage();
        return currentAverage;
    }

    private float calcAverage() {
        float total = 0;
        int maxIndex = firstTimeAround ? cursor : samples.length;
        for (int i = 0; i < maxIndex; i++) {
            total += samples[i];
        }
        return total / maxIndex;
    }
    
    public float getCurrentValue() {
        return currentAverage;
    }
}
