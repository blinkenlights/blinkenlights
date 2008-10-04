package jmo.breakout;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Point;

public class Ball implements BreakoutObject {
	
	private static final double INITIAL_VELOCITY = 1;
	
	private static final double INITIAL_ANGLE = 0.25 * Math.PI;
	
	private double angle;
	
	private double velocity;
	
	private boolean isStuckOnPlayer;
	
	
	/**
	 * Central x co-ordinate
	 */
	private double x;
	
	/**
	 * Central y co-ordinate
	 */
	private double y;
	
	public Ball(int x, int y) {
		this.x = x;
		this.y = y;
		this.angle = INITIAL_ANGLE;
		this.velocity = 0;
		isStuckOnPlayer = true;
	}
	
	public void move(int x, int y) {
		this.x += x;
		this.y += y;
	}

	public int getHeight() {
		return 1;
	}

	public int getWidth() {
		return 1;
	}

	public void paint(Graphics g) {
		String stringX = Long.toString(Math.round(x));
		String stringY = Long.toString(Math.round(y));
		
		g.setColor(new Color(255, 255, 255));
		g.drawLine(Integer.valueOf(stringX),
				   Integer.valueOf(stringY),
				   Integer.valueOf(stringX), 
				   Integer.valueOf(stringY));
	}
	
	public void calculateNextPosition() {
		double deltaX = Math.cos(angle) * velocity;
		double deltaY = Math.sin(angle) * velocity;

		x += deltaX;
		y += deltaY;
	}

	public Point approximateNextPoint() {
		double deltaX = Math.cos(angle) * velocity;
		double deltaY = Math.sin(angle) * velocity;
		
		String stringX = Long.toString(Math.round(x + deltaX));
		String stringY = Long.toString(Math.round(y + deltaY));
		
		return new Point(Integer.valueOf(stringX), Integer.valueOf(stringY));
	}
	
	public double getAngle() {
		return angle;
	}
	
	public void setAngle(double angle) {
		this.angle = angle;
	}
	
	public double getX() {
		return x;
	}
	
	public int getXAsInt() {
		return Integer.valueOf(Long.toString(Math.round(x)));
	}
	
	public double getY() {
		return y;
	}
	
	public int getYAsInt() {
		return Integer.valueOf(Long.toString(Math.round(y)));
	}

	public boolean isStuckOnPlayer() {
		return isStuckOnPlayer;
	}
	
	public void setStuckOnPlayer(boolean isStuckOnPlayer) {
		this.isStuckOnPlayer = isStuckOnPlayer;
		if (!isStuckOnPlayer) {
			velocity = INITIAL_VELOCITY;
		}
	}
}

