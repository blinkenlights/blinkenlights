package breakout;

import java.awt.Graphics2D;
import java.awt.Point;
import java.util.ArrayList;
import java.util.List;

import spaceInvaders.SpaceInvaders;
import util.Util;

import de.blinkenlights.game.BlinkenGame;
import de.blinkenlights.game.FrameInfo;
import de.blinkenlights.game.GameConfigurationException;
import de.blinkenlights.game.GameContext;

/**
 * Breakout! All points start from the left most position. Imagine
 * the screen as if the paddle is at the bottom. Sorry for any
 * maintainers, I seem to have odd logic between this and the direction
 * left and up in the Ball class.
 */
public class Breakout implements BlinkenGame {
	
	private static final int STARTING_BRICK_LENGTH = 3;
	private static final int PADDLE_LENGTH = 3;
	private static final int START_ROWS = 3;
	
	private int maxWidth;
	private int maxHeight;
	private Paddle paddle;
	private Ball ball;
	private List<Brick> bricks;
	private int currentBrickLength;
	private int currentBrickRows;
	private int score;

	public void gameEnding(Graphics2D g) {
		Util.paintGameOver(g, score);
	}

	public void gameStarting(GameContext context) {
		context.setFramesPerSecond(5);
		maxWidth = context.getPlayfieldWidth();
		maxHeight = context.getPlayfieldHeight();
		
		paddle = new Paddle(this, new Point(maxWidth-1, maxHeight/2+1), PADDLE_LENGTH);
		ball = new Ball(this, new Point((int) (maxWidth/2 + Math.random()*maxWidth/4), (int) (Math.random()*(maxHeight-2)+1)));
		score = 0;
		currentBrickRows = START_ROWS;
		
		currentBrickLength = STARTING_BRICK_LENGTH;
		bricks = new ArrayList<Brick>();
		createBricks();
	}

    private void createBricks() {
        for (int i = 0; i < START_ROWS; i++) {
			for (int j = 0; j < (maxHeight)/currentBrickLength; j++) {
				bricks.add(new Brick(new Point(i, j*(currentBrickLength) + currentBrickLength - 1), currentBrickLength));
			}
		}
    }

	public boolean nextFrame(Graphics2D g, FrameInfo info) {
		Character keyPress = info.getUserInput();
		if (keyPress != null) {
			if (keyPress == '2') {
				paddle.move(true);
			} else if (keyPress == '8') {
				paddle.move(false);
			}
		}
		
		for (Point p : paddle.getPaddlePoints()) {
			if (ball.getLocation().x + 1 == p.x && ball.getLocation().y == p.y) {
				ball.paddleHit();
				break;
			}
		}
		
		List<Brick> hitBricks = new ArrayList<Brick>();
		for (Brick b : bricks) {
			for (Point p : b.getPoints()) {
				if (ball.getLocation().x - 1 == p.x && ball.getLocation().y == p.y) {
					ball.brickHit();
					hitBricks.add(b);
					score++;
					break;
				}
				if (ball.getLocation().x == p.x && ball.getLocation().y - 1 == p.y) {
					ball.brickHitFromBelow();
					hitBricks.add(b);
					score++;
					break;
				}
				if (ball.getLocation().x == p.x && ball.getLocation().y + 1 == p.y) {
					ball.brickHitFromBelow();
					hitBricks.add(b);
					score++;
					break;
				}
			}
		}
		
		for (Brick b : hitBricks) {
			bricks.remove(b);
		}
		
		if (bricks.isEmpty()) {
			currentBrickRows++;
			createBricks();
		}
		
		ball.move();
		
		if (ball.getLocation().x == maxWidth) {
			return false;
		}
			
		paddle.paint(g);
		ball.paint(g);
		for (Brick b : bricks) {
			b.paint(g);
		}
		return true;
	}

	public int getMaxWidth() {
		return maxWidth;
	}

	public int getMaxHeight() {
		return maxHeight;
	}
	
    public static void main(String[] args) throws GameConfigurationException {
        Breakout game = new Breakout();
        GameContext context = new GameContext(game);
        context.start();
    }

}
