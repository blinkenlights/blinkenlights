package de.blinkenlights.bmix.util;

import java.awt.Dimension;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.Arrays;

import javax.imageio.ImageIO;

import de.blinkenlights.bmix.movie.BMLOutputStream;

public class PNGtoBML {

	/**
	 * @param args
	 */
	public static void main(String[] args) throws IOException {
		
		if (args.length != 2) {
			System.out.println("usage: <png directory> <output file>");
		}
		File pngDir = new File(args[0]);
		if (!pngDir.isDirectory()) {
			System.out.println("png directory wasn't a directory");
		}
		File outFile = new File(args[1]);
		OutputStream out = new FileOutputStream(outFile);
		File[] files = pngDir.listFiles();
		Arrays.sort(files);
		BufferedImage tempImage=(BufferedImage)ImageIO.read(files[0]);
		System.out.println("Creating BML based on dimensions of first file: "+tempImage.getWidth()+"x"+tempImage.getHeight());
		BMLOutputStream bmlOut = new BMLOutputStream(out,10,new Dimension(tempImage.getWidth(),tempImage.getHeight()),4);
		
		for (File file : pngDir.listFiles()) {
			BufferedImage image=(BufferedImage)ImageIO.read(file);
			if (image != null) {
				bmlOut.writeFrame(image);
			} else {
				System.out.println("file could not be imported: "+file.getName());
			}
		}
		bmlOut.close();
	}
}
