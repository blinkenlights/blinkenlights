package snake;

import java.awt.Graphics;
import java.awt.Point;
import java.util.ArrayList;

public class TheSnake {

	private ArrayList<Point> snakePoints;
	
	public enum Direction {LEFT, RIGHT, UP, DOWN};
	
	public enum SnakeState {MOVING, EATEN, DEAD};
	
	private Direction movingDirection;
	
	private Snake game;
	
	public TheSnake(Snake s, ArrayList<Point> startingPositions) {
		snakePoints = startingPositions;
		movingDirection = Direction.LEFT;
		game = s;
	}
	
	/**
	 * Enter the position of the food and the snake will move and eat
	 * the food if it is in front of itself. Returns if the snake is moving,
	 * has eaten food, or ate itself. If the snake has eaten then the food
	 * is gone and the snake is still moving.
	 */
	public SnakeState move(Point foodPosition) {
		Point head = snakePoints.get(0);
		if (movingDirection == Direction.LEFT && head.x - 1 == foodPosition.x && head.y == foodPosition.y ||
				movingDirection == Direction.RIGHT && head.x + 1 == foodPosition.x && head.y == foodPosition.y  ||
				movingDirection == Direction.UP && head.y - 1 == foodPosition.y && head.x == foodPosition.x  ||
				movingDirection == Direction.DOWN && head.y + 1 == foodPosition.y && head.x == foodPosition.x ){
			snakePoints.add(0, foodPosition);
			return SnakeState.EATEN;
		} else {
			if (movingDirection == Direction.LEFT) {
				snakePoints.add(0, new Point(head.x - 1, head.y));
			} else if (movingDirection == Direction.RIGHT) {
				snakePoints.add(0, new Point(head.x + 1, head.y));
			} else if (movingDirection == Direction.UP) {
				snakePoints.add(0, new Point(head.x, head.y - 1));
			} else if (movingDirection == Direction.DOWN) {
				snakePoints.add(0, new Point(head.x, head.y + 1));
			} 
			snakePoints.remove(snakePoints.size() - 1);
		}

		if (snakePoints.get(0).x < 0 || snakePoints.get(0).x > game.getMaxWidth() || snakePoints.get(0).y < 0 || snakePoints.get(0).y > game.getMaxHeight()) {
			return SnakeState.DEAD;
		}
		for (Point p : snakePoints) {
			for (Point q : snakePoints) {
				if (p.equals(q) && p != q) {
					return SnakeState.DEAD;
				}
			}
		}
		
		return SnakeState.MOVING;
	}
	
	public void changeDirection(Direction dir) {
		movingDirection = dir;
	}
	
	public void paint(Graphics g) {
		for (Point p : snakePoints) {
			g.drawLine(p.x, p.y, p.x, p.y);
		}
	}
}
