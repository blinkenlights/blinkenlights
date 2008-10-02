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
package de.blinkenlights.bmix.mixer;

import java.awt.Rectangle;
import java.io.IOException;
import java.util.List;

import de.blinkenlights.bmix.mixer.AbstractOutput.PacketType;
import de.blinkenlights.bmix.network.HostAndPort;

public interface Output {

    /**
     * Reads the data in this output's viewport rectangle from the given BLImage
     * and sends it to all registered recipients.
     * 
     * @throws IOException if there was an error sending the packet
     */
    public abstract void send() throws IOException;

    /**
     * Adds a new screen to this output. Only the MCU_MULTIFRAME output
     * type supports more than one screen.
     * 
     * @param bounds
     * @param bpp bits per pixel. Ignored unless this is an MCU_MULTIFRAME output.
     * @param screenId multiframe screen id.  Ignored unless this is an MCU_MULTIFRAME output.
     */
    public abstract void addScreen(Rectangle bounds, int bpp, int screenId);

    public abstract List<BLImageViewport> getViewports();

    public abstract PacketType getPacketType();

    public abstract long getMinSendInterval();

    public abstract long getLastSendTime();

    public abstract List<HostAndPort> getDestinations();

    /**
     * Closes this output, releasing any system resources such as file
     * and network handles. Once closed, this output instance cannot
     * be reused.
     */
    public void close();
}