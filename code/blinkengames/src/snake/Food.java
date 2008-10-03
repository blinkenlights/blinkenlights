package snake;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Point;

public class Food {
	
	public static final int DARKEST_SHADE = 5;

	private Point food;
	
	/**
	 * Should only go from 0 to 15 although 15 probably isn't very visible.
	 */
	private int colour;
	
	private boolean darkening = true;
	
	private Snake game;
	
	public Food(Snake s) {
		colour = 15;
		game = s;
		food = new Point((int)(Math.random() * (game.getMaxWidth() - 2)) + 1, (int)(Math.random() * (game.getMaxHeight() - 2)) + 1);
	}
	
	public Point getFoodPosition() {
		return food;
	}
	
	public void paint(Graphics g) {
		g.setColor(new Color(colour * 16, colour * 16, colour * 16));
		g.drawLine(food.x, food.y, food.x, food.y);
		
		if (!darkening) {
			colour++;
			if (colour == 15) {
				darkening = true;
			}
		} else if (darkening) {
			colour--;
			if( colour == DARKEST_SHADE) {
				darkening = false;
			}
		}
	}
}
