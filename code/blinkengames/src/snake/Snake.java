package snake;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Point;
import java.util.ArrayList;

import util.Util;
import de.blinkenlights.game.BlinkenGame;
import de.blinkenlights.game.FrameInfo;
import de.blinkenlights.game.GameConfigurationException;
import de.blinkenlights.game.GameContext;

public class Snake implements BlinkenGame {

	/**
	 * The max width of the screen.
	 */
	private int maxWidth;
	
	/**
	 * The max height of the screen.
	 */
	private int maxHeight;
	
	private TheSnake snake;
	
	private Food food;
	
	private int score;
	
	public Snake() {
	}
	
	public void paint(Graphics g) {
		
		g.clearRect(0, 0, maxWidth, maxHeight);
		g.setColor(Color.WHITE);
		snake.paint(g);
		
		food.paint(g);
	}
	
	/**
	 * The method that should be fed keys from the user. This will
	 * handle all user input.
	 */
	public void registerKeyPress(Character key) {
		if (key == null) {
			return;
		} else if (key == '4') {
			snake.changeDirection(TheSnake.Direction.LEFT);
		} else if (key == '6') {
			snake.changeDirection(TheSnake.Direction.RIGHT);
		} else if (key == '8') {
			snake.changeDirection(TheSnake.Direction.DOWN);
		} else if (key == '2') {
			snake.changeDirection(TheSnake.Direction.UP);
		} else if (key == '5') {
		} else {
			System.out.println("Invalid Key: " + key);
		}
	}
	
	/////////////////////////////Main method for testing///////////////
	
    public static void main(String[] args) throws GameConfigurationException {
        Snake game = new Snake();
        GameContext context = new GameContext(game);
        context.start();
    }

	
	public int getMaxWidth() {
		return maxWidth;
	}
	
	public int getMaxHeight() {
		return maxHeight;
	}

	public void gameEnding(Graphics2D g) {
		Util.paintGameOver(g, score);
	}

	public void gameStarting(GameContext context) {
		maxHeight = context.getPlayfieldHeight();
		maxWidth = context.getPlayfieldWidth();
		
		context.setFramesPerSecond(3);
		
		ArrayList<Point> snakeStart = new ArrayList<Point>();
		Point snakeHead = new Point(maxWidth/2, maxHeight/2);
		snakeStart.add(snakeHead);
		for (int i = 1; i < 5; i++) {
			snakeStart.add(new Point((int)snakeHead.getX() + i, (int)snakeHead.getY()));
		}
		snake = new TheSnake(this, snakeStart);
		
		food = new Food(this);
		
		score = 0;
	}

	public boolean nextFrame(Graphics2D g, FrameInfo info) {
		registerKeyPress(info.getUserInput());
		
		TheSnake.SnakeState snakeState = snake.move(food.getFoodPosition());
		if (snakeState == TheSnake.SnakeState.EATEN) {
			food = new Food(this);
			score++;
			System.out.println("score " + score);
			if (score == 99) {
				//max score printable;
				return false;
			}
		} else if (snakeState == TheSnake.SnakeState.DEAD) {
			System.out.println("YOU'RE DEAD!");
			return false;
		}
		
		paint(g);
		
		return true;
	}
}
