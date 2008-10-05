package jmo.breakout;

import java.awt.Color;
import java.awt.Graphics;

public class Player implements BreakoutObject {

	private static final int PLAYER_HEIGHT = 0;
	private static final int PLAYER_WIDTH = 4;
	
	/**
	 * Central x co-ordinate
	 */
	private int x;
	
	/**
	 * Central y co-ordinate
	 */
	private int y;
	
	public Player(int x, int y) {
		this.x = x;
		this.y = y;
	}
	
	public void move(int x, int y) {
		// player cannot change height so ignore y.
		this.x += x;
	}

	public int getHeight() {
		return PLAYER_HEIGHT;
	}

	public int getWidth() {
		return PLAYER_WIDTH;
	}

	public void paint(Graphics g) {
		g.setColor(new Color(255, 255, 255));
		g.drawLine(x, y, x + PLAYER_WIDTH, y + PLAYER_HEIGHT);
	}
	
	public int getX() {
		return x;
	}
	
	public int getY() {
		return y;
	}
}
