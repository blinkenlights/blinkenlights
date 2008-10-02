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
package de.blinkenlights.bmix.util;

import org.xml.sax.SAXException;

/**
 * This class is an exception that is thrown when a config file has an
 * error and cannot be parsed.
 */
public class FileFormatException extends SAXException {
	private static final long serialVersionUID = 1L;

	/**
     * Creates a new FileFormatException, which is suited to reporting problems with
     * parsing the content of a file.
     * 
     * @param message The error message to show to the user.
     * @param lineNum The line number in the input file that was unparseable.  The
     * special value -1 indicates end-of-file.
     * @param badCharPos The character position of the offending data, if available.
     * The special value -1 means the whole line. 
     */
    public FileFormatException(String message, int lineNum, int badCharPos) {
        super(lineNum + ":" + badCharPos + ": " + message);
    }

    /**
     * Creates a new FileFormatException, which is suited to reporting problems with
     * parsing the content of a file.
     * 
     * @param message The error message to show to the user.
     * @param lineNum The line number in the input file that was unparseable.  The
     * special value -1 indicates end-of-file.
     * @param badCharPos The character position of the offending data, if available.
     * The special value -1 means the whole line. 
     * @param cause the exception that caused the parse to fail
     */
    public FileFormatException(int lineNum, int badCharPos, Throwable cause) {
        super(lineNum + ":" + badCharPos + ": " + cause);
        initCause(cause);
    }

}