package jmo.breakout;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Point;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import jmo.util.Util;
import de.blinkenlights.game.BlinkenGame;
import de.blinkenlights.game.FrameInfo;
import de.blinkenlights.game.GameConfigurationException;
import de.blinkenlights.game.GameContext;

public class Breakout implements BlinkenGame {

	public static final Color DEFAULT_OBJECT_COLOR = new Color(255, 255, 255);

	int score = 0;
	
	int worldWidth = 0;
	
	int worldHeight = 0;
	
	Player player;
	
	Ball ball;
	
	List<BreakoutObject> objects = new ArrayList<BreakoutObject>();
	
	Map<Point, Block> blocks = new HashMap<Point, Block>();
	
	public void paint(Graphics g) {
		g.clearRect(0, 0, worldWidth, worldHeight);
		g.setColor(Color.WHITE);
		
		for (BreakoutObject b: objects) {
			b.paint(g);
		}
		
		for (Block b: blocks.values()) {
			b.paint(g);
		}
	}
	
	public void gameEnding(Graphics2D g) {
		Util.paintGameOver(g, score);
	}

	public void gameStarting(GameContext context) {
		worldHeight = context.getPlayfieldHeight();
		worldWidth = context.getPlayfieldWidth();
		
		context.setFramesPerSecond(3);
		
		player = new Player(worldWidth / 2, worldHeight - 1);
		
		ball = new Ball(player.getX() + 3, worldHeight - 2);

		score = 0;
	
		objects.clear();
		objects.add(player);
		objects.add(ball);
		
		blocks.clear();
		
		Color oddBlockColor = new Color(170, 170, 170);
		Color evenBlockColor = new Color(85, 85, 85);
		
		for (int i = 0; i < 5 ; i++) {
			for (int j = 0; j * Block.BLOCK_WIDTH < worldWidth; j ++) {
				Block newBlock = null;
				
				if (i % 2 != 0) {
					newBlock = new Block(j * Block.BLOCK_WIDTH, i, (j % 2 != 0) ? oddBlockColor : evenBlockColor);
				} else {
					newBlock = new Block(j * Block.BLOCK_WIDTH, i, (j % 2 != 0) ? evenBlockColor : oddBlockColor);
				}

				for (Point p: newBlock.getPoints()) {
					blocks.put(p, newBlock);
				}
			}
		}
		
	}

	public boolean nextFrame(Graphics2D g, FrameInfo info) {
		// Ball fell out of play
		if (Math.round(ball.getY()) >= worldHeight) {
			return false;
		}
		
		registerKeyPress(info.getUserInput());
		
		calculateCollisions();
		
		ball.calculateNextPosition();
		
		paint(g);
		
		return true;
	}

	private void calculateCollisions() {
		
		Point p = ball.approximateNextPoint();		
		int deltaX = p.x - ball.getXAsInt();
		int deltaY = p.y - ball.getYAsInt();
		
		Block b = null;
		
		if (deltaY < 0) {
			b = blocks.get(new Point(ball.getXAsInt(), ball.getYAsInt() - 1));
			if (b!= null) {
				destroyBlock(b);
				ball.setAngle(2*Math.PI - ball.getAngle());
				return;
			}
		} else if (deltaY > 0){
			b = blocks.get(new Point(ball.getXAsInt(), ball.getYAsInt() + 1));
			if (b!= null) {
				destroyBlock(b);
				ball.setAngle(2*Math.PI - ball.getAngle());
				return;
			}
		}
		
		if (deltaX < 0) {
			b = blocks.get(new Point(ball.getXAsInt() - 1, ball.getYAsInt()));
			if (b!= null) {
				destroyBlock(b);
				ball.setAngle(Math.PI - ball.getAngle());
				return;
			}
		} else if (deltaX > 0){
			b = blocks.get(new Point(ball.getXAsInt() + 1, ball.getYAsInt()));
			if (b!= null) {
				destroyBlock(b);
				ball.setAngle(Math.PI - ball.getAngle());
				return;
			}
		}
		
		
		// If ball hits player
		if ((Math.round(ball.getY()) >= player.getY() - 1) && (Math.round(ball.getX()) >= player.getX() && Math.floor(ball.getX()) <= player.getX() + player.getWidth())) {
			if (Math.sin(ball.getAngle()) >= 0) {
				if (ball.getXAsInt() == player.getX()) {
					ball.setAngle(1.17 * Math.PI);
				} else if (ball.getXAsInt() == player.getX() + 1) {
					ball.setAngle(1.33 * Math.PI);
				} else if (ball.getXAsInt() == player.getX() + 2) {
					ball.setAngle(1.5 * Math.PI);
				} else if (ball.getXAsInt() == player.getX() + 3) {
					ball.setAngle(1.67 * Math.PI);
				} else if (ball.getXAsInt() == player.getX() + 4) {
					ball.setAngle(1.83 * Math.PI);
				}
				
			}
		} else if (Math.round(ball.getY()) <= 0) {
			if (Math.round(ball.getX()) <= 0 || Math.round(ball.getX()) + 1 >= worldWidth) {
				ball.setAngle(ball.getAngle() + Math.PI);
			} else {
				if (Math.sin(ball.getAngle()) < 0) {
					ball.setAngle(2*Math.PI - ball.getAngle());
				}
			}
		} else if (Math.round(ball.getX()) <= 0 || Math.round(ball.getX()) + 1 >= worldWidth) {
			ball.setAngle(Math.PI - ball.getAngle());
		}
	}

	private void destroyBlock(Block b) {
		for (Point blockPoints: b.getPoints()) {
			blocks.remove(blockPoints);
		}
		score++;
	}

	public static void main (String[] args) throws GameConfigurationException {
		Breakout game = new Breakout();
        GameContext context = new GameContext(game);
        context.start();
	}
	
	/**
	 * The method that should be fed keys from the user. This will
	 * handle all user input.
	 */
	public void registerKeyPress(Character key) {
		if (key == null) {
			return;
		} else if (key == '4') {
			if (player.getX() > 0) {
				player.move(-1, 0);
				if (ball.isStuckOnPlayer()) {
					ball.move(-1, 0);
				}
			}
		} else if (key == '6') {
			if (player.getX() + player.getWidth() <= worldWidth) {
				player.move(1, 0);
				if (ball.isStuckOnPlayer()) {
					ball.move(1, 0);
				}
			}
		} else if (key == '2') {
			ball.setStuckOnPlayer(false);
		} else {
			System.out.println("Invalid Key: " + key);
		}
	}
}
