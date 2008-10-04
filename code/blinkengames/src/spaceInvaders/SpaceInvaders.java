package spaceInvaders;

import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.List;

import javax.swing.Timer;

import util.Util;

import de.blinkenlights.game.BlinkenGame;
import de.blinkenlights.game.FrameInfo;
import de.blinkenlights.game.GameConfigurationException;
import de.blinkenlights.game.GameContext;

public class SpaceInvaders implements BlinkenGame {
	
	private static final int START_ALIEN_DELAY = 3;
	private static final int START_ALIEN_ROWS = 2;
	private static final int GUN_COOLDOWN = 3;

	private boolean switchDirection = true;
	
	private Ship player;
	
	private List<Bullet> bullets;
	
	private List<Alien> aliens;
	
	private List<Explosion> explosions;

	private int maxWidth;

	private int maxHeight;
	
	private int score;
	
	private int lives;
	
	private int currentAlienDelay;
	private int numAlienRows;
	private int gunCooldown;
	
	public SpaceInvaders() {
	}
	
	public void paint(Graphics g) {
		
		g.clearRect(0, 0, maxWidth, maxHeight);
		player.paint(g);
		for (Bullet b : bullets) {
			b.paint(g);
		}
		
		for (Alien a : aliens) {
			a.paint(g);
		}
		
		for (Explosion e : explosions) {
			e.paint(g);
		}
	}
	
	/**
	 * The method that should be fed keys from the user. This will
	 * handle all user input.
	 */
	public void registerKeyPress(Character key) {
		if (key == null) {
			return;
		}
		if (key == '2') {
			player.moveLeft();
		} else if (key == '8') {
			player.moveRight();
		} else if (key == '4') {
			if (gunCooldown <= 0) {
				bullets.add(new Bullet(new Point(maxWidth - 1, player.getLocation())));
				gunCooldown = GUN_COOLDOWN;
			}
		} else if (key == '5') {
		} else {
			System.out.println("Invalid Key: " + key);
		}
	}
	
	/////////////////////////////Main method for testing///////////////
	
    public static void main(String[] args) throws GameConfigurationException {
        SpaceInvaders game = new SpaceInvaders();
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
		maxWidth = context.getPlayfieldWidth();
		maxHeight = context.getPlayfieldHeight();
		
		context.setFramesPerSecond(3);
		
		score = 0;
		lives = 1;
		currentAlienDelay = START_ALIEN_DELAY;
		numAlienRows = START_ALIEN_ROWS;
		gunCooldown = 0;
		
		switchDirection = true;
		player = new Ship(this);
		bullets = new ArrayList<Bullet>();
		aliens = new ArrayList<Alien>();
		for (int i = 0; i < numAlienRows; i++) {
			for (int j = 0; j < (maxHeight)/2; j++) {
				if (i%2 == j%2) {
					aliens.add(new Alien(new Point(1 + i*2, 1 + j*2), this, currentAlienDelay));
				}
			}
		}
		explosions = new ArrayList<Explosion>();
		
	}

	public boolean nextFrame(Graphics2D g, FrameInfo info) {
		gunCooldown--;
		registerKeyPress(info.getUserInput());
		
		List<Bullet> deadBullets = new ArrayList<Bullet>();
		for (Bullet b : bullets) {
			b.moveBullet();
			if (b.getLocation().y < 0) {
				deadBullets.add(b);
			}
		}
		for (Bullet b : deadBullets) {
			bullets.remove(b);
		}
		
		List<Alien> deadAliens = new ArrayList<Alien>();
		boolean switchDir = false;
		for (Alien a : aliens) {
			boolean newSwitch = a.move(switchDirection);
			switchDir = switchDir || newSwitch;
			if (a.getLocation().x >= maxWidth) {
				lives--;
				if (lives == 0) {
					return false;
				}
				deadAliens.add(a);
			} else if (a.getLocation().y < 0 || a.getLocation().y >= maxHeight) {
				deadAliens.add(a);
			}
		}
		switchDirection = switchDir;
		
		List<Bullet> expiredBullets = new ArrayList<Bullet>();
		for (Bullet b : bullets) {
			for (Alien a : aliens) {
				for (int i = 0; i < Bullet.BULLET_SPEED; i++) {
					if (b.getLocation().x - i == a.getLocation().x && b.getLocation().y == a.getLocation().y) {
						deadAliens.add(a);
						expiredBullets.add(b);
						score++;
						if (score == 99) {
							return false;
						}
						System.out.println("Score " + score);
						break;
					}
				}
			}
		}
		for (Alien a : deadAliens) {
			aliens.remove(a);
			explosions.add(new Explosion(a.getLocation()));
		}
		
		if (aliens.isEmpty()) {
			if (currentAlienDelay > 0) {
				currentAlienDelay--;
			}
			numAlienRows++;
			for (int i = 0; i < numAlienRows; i++) {
				for (int j = 0; j < (maxHeight)/2; j++) {
					if (i%2 == j%2) {
						aliens.add(new Alien(new Point(1 + i*2, 1 + j*2), this, currentAlienDelay));
					}
				}
			}
		}
		
		for (Bullet b : expiredBullets) {
			bullets.remove(b);
		}
		
		List<Explosion> doneExplosions = new ArrayList<Explosion>();
		for (Explosion e : explosions) {
			if (e.isDone()) {
				doneExplosions.add(e);
			}
		}
		for (Explosion e : doneExplosions) {
			explosions.remove(e);
		}
		
		paint(g);
		return true;
	}
}
