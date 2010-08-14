/* 
 * Created on Aug 13, 2010
 *
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
package de.blinkenlights.bmix.mixer;

import java.awt.RenderingHints;

public enum ScaleMode {

    NEAREST_NEIGHBOUR("nearest-neighbour", RenderingHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR),
    BILINEAR("bilinear", RenderingHints.VALUE_INTERPOLATION_BILINEAR),
    BICUBUIC("bicubic", RenderingHints.VALUE_INTERPOLATION_BICUBIC);
    
    private final Object renderingHintValue;
    private final String xmlCode;

    private ScaleMode(String xmlCode, Object renderingHintValue) {
        this.xmlCode = xmlCode;
        this.renderingHintValue = renderingHintValue;
    }
    
    public Object getRenderingHintValue() {
        return renderingHintValue;
    }
    
    public static ScaleMode forCode(String code) {
        for (ScaleMode sm : values()) {
            if (sm.xmlCode.equals((code))) {
                return sm;
            }
        }
        throw new IllegalArgumentException("Unknown scale mode: '" + code + "'");
    }
}
