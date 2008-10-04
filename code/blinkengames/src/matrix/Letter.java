package matrix;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Point;

public class Letter {
	
	public static final int LIGHTEST_SHADE = 15;

	private Point letter;
	
	/**
	 * Should only go from 0 to 15 although 15 probably isn't very visible.
	 */
	private int colour;
	
	/**
	 * Controls the basic lightening and darkening of a single letter.
	 */
	private boolean darkening;
	
	/**
	 * Count to adjust the track the number of cycles a letter has lived.
	 */
	private int age = 0;
	
	/**
	 * The maximum age a letter can reach before being deleted.
	 */
	private static final int LIFESPAN = 25;
	
	public Letter() {
		colour = 0;
		letter = new Point((int)(Math.random() * (Matrix.MAX_WIDTH - 2)) + 1, (int)(Math.random() * (Matrix.MAX_HEIGHT - 2)/2) + 1);
		darkening = false;
	}
	
	public Letter(int x, int y, int i) {
		colour = 0 + i;
		letter = new Point(x, (y + 1) % Matrix.MAX_HEIGHT);
		darkening = false;
	}
	
	public void paint(Graphics g, Point p) {
		g.setColor(new Color(colour * 16, colour * 16, colour * 16));
		g.drawLine(p.x, p.y, p.x, p.y);
				
		if (!darkening) {
			if (age == 0 ) {
				colour += 4;
				age++;
			}
			colour++;
			age++;
			if (colour == LIGHTEST_SHADE) {
				darkening = true;
			}
		} else if (darkening) {
			
			colour--;
			age++;
			if( colour == 0) {
				darkening = false;
			}
		}
	}
	
	public Point getLetterPosition() {
		return letter;
	}
	
	public boolean isLetterDarkening() {
		return darkening;
	}
	public boolean isDone() {
		return age > LIFESPAN;
	}
}
