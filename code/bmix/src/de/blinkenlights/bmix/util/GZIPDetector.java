package de.blinkenlights.bmix.util;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Arrays;

public class GZIPDetector {
	
	
	public static boolean isGZIPFile(File file) throws IOException {
		byte header[] = new byte[2];
		FileInputStream in = new FileInputStream(file);
		in.read(header);
		in.close();
		System.out.println(Arrays.toString(header));
		// gzip magic header
		if ((header[0] & 0xFF) == 0x1f && (header[1] & 0xFF) == 0x8b) {
			return true;
		} 
		return false;
	}
}
