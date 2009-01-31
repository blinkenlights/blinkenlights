/* 
 * Created on Jan 31, 2009
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

import java.io.IOException;

import de.blinkenlights.bmix.network.BLNetworkException;

public interface OutputSender {

    /**
     * Releases network resources. Once this method has been called, this
     * sender can no longer be used.
     */
    public void close();
    
	/**
	 * Sends a message to the client.
	 * 
	 * @param buf the buffer to send
	 * @throws BLNetworkException if there was an error sending the packet
	 */
	public void send(byte buf[]) throws IOException;
	
	/**
	 * Sends a message to the client.
	 * 
	 *  @param buf the buffer to send
	 *  @param offset the offset in the buffer
	 *  @param length the length in the buffer
	 * @throws IOException 
	 */
	public void send(byte buf[], int offset, int length) throws IOException;

	
	public int getPort();

	public String getAddress();	
}
