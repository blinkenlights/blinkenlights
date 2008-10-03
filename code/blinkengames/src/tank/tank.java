	package tank;
	
	import java.awt.Graphics;
	import java.awt.Graphics2D;
	import java.awt.Point;
	import java.util.List;
	import java.util.ArrayList;
	
	import spaceInvaders.Explosion;
	import util.Util;
	import de.blinkenlights.game.BlinkenGame;
	import de.blinkenlights.game.FrameInfo;
	import de.blinkenlights.game.GameConfigurationException;
import de.blinkenlights.game.GameContext;
	
	public class tank implements BlinkenGame {
		
		private int maxWidth;
		private int maxHeight;
		private int score;
		private int lives;
		private theTank player;
		private List<Bullet> bullets;
		private List<Explosion> booms;
		private List<Point> walls; 
		private List<theTank> evilTanks;
		private int level;
		
		public tank(){
			walls = new ArrayList<Point>();
		}
		
		public List<Point> getWalls(){
			return walls;
		}
	
		/**
		 * Paints the game over screen
		 */
		public void gameEnding(Graphics2D g) {
			Util.paintGameOver(g, score);
	
		}
	
		/**
		 *  creates the tanks and the map as well as their starting positions
		 */
		public void gameStarting(GameContext context) {
			maxWidth = context.getPlayfieldWidth();
			maxHeight = context.getPlayfieldHeight();
			evilTanks = new ArrayList<theTank>();
			lives = 2;
			level = 1;	
			
			context.setFramesPerSecond(3);
			
			score = 0;
			lives = 2;
			
			player = new theTank(this);
			
			bullets = new ArrayList<Bullet>();
			
			booms = new ArrayList<Explosion>();
			
			evilTanks.add(new theTank(this, new Point(0,0),level));
			evilTanks.add(new theTank(this, new Point(getMaxWidth()-3,getMaxHeight()-3),level));
			evilTanks.add(new theTank(this, new Point(getMaxWidth()-3,0),level));
	
		}
		
		public void paint(Graphics g) {
			
			g.clearRect(0, 0, maxWidth, maxHeight);
			createMap(g);
			player.paint(g);
			for (Bullet b : bullets) {
				b.paint(g);
			}
			
			for(theTank t : evilTanks){
				t.paint(g);
			}
			
			
			for (Explosion e : booms) {
				e.paint(g);
			}
		}
		/**
		 * creates the map layout for a very small maze effect
		 * @param g
		 */
		private void createMap(Graphics g){
			//top left wall
			g.drawLine(3, 3, 6, 3);
			g.drawLine(3, 3, 3, 6);	
			
			//bottom left wall
			g.drawLine(3, getMaxHeight()-4, 3, getMaxHeight()-7);
			g.drawLine(3, getMaxHeight()-4, 6, getMaxHeight()-4);
			
			//bottom right wall
			g.drawLine(getMaxWidth()-4, getMaxHeight()-4, getMaxWidth()-7, getMaxHeight()-4);
			g.drawLine(getMaxWidth()-4, getMaxHeight()-4, getMaxWidth()-4, getMaxHeight()-7);
			
			//top right wall
			g.drawLine(getMaxWidth()-4, 3, getMaxWidth()-7, 3);
			g.drawLine(getMaxWidth()-4, 3, getMaxWidth()-4, 6);
			
			for(int i = 3; i <= 6; i ++) {
				//top left wall
				walls.add(new Point(3,i));
				walls.add(new Point(i,3));
				
				//bottom right wall
				walls.add(new Point(getMaxWidth()-1-i,getMaxHeight()-4));
				walls.add(new Point(getMaxWidth()-1-i,getMaxHeight()-4));
				
				//top right wall
				walls.add(new Point(getMaxWidth()-1-i, 3));
				walls.add(new Point(getMaxWidth()-4,i));
				
				//bottom left wall
				walls.add(new Point(3, getMaxHeight()-1-i));
				walls.add(new Point(i,getMaxHeight()-4));
			}
			
			
		}
		
	
		public boolean nextFrame(Graphics2D g, FrameInfo info) {
			registerKeyPress(info.getUserInput());
			
			List<Bullet> deadBullets = new ArrayList<Bullet>();
		    List<theTank> deadTanks =  new ArrayList<theTank>();
			
			//checks if the bullets hit any walls
			for (Bullet b : bullets) {
				b.moveBullet();
				if (b.getLocation().y < 0 || b.getLocation().x < 0 
					|| b.getLocation().x > this.getMaxWidth()
					|| b.getLocation().y > this.getMaxHeight() 
					|| getWalls().contains(b.getLocation())) {
	
					deadBullets.add(b);
				}
			}
			
			//check if player has been shot
			for(Bullet b : bullets){
				if(player.getLocation().contains(b.getLocation())){
					deadBullets.add(b);
					booms.add(new Explosion(b.getLocation()));
					player = new theTank(this);
					lives --;
				}
				
			}
			// checks if the bullets hit the enemy tanks
			for(Bullet b : bullets){
				for(theTank t : evilTanks){
					if(t.getLocation().contains(b.getLocation()) && b.getOwner().equals(player)){
						t.damged();
							if(t.getHp() == 0){
								deadTanks.add(t);
								booms.add(new Explosion(b.getLocation()));
								score++;
							}
							deadBullets.add(b);
					}
				}
			}
			
			for(theTank t : evilTanks){
				int fireChance = (int)(Math.random() * 3);
				if(fireChance < 2){
					bullets.add(new Bullet(t.getCannonLocation(), t.getLastMove(), t));
				}
				startRoutePath(t);

			}
			
			
			
			//if tank runs out of lives end game
			if(lives == 0){
				return false;
			}
			
			
			List<Explosion> doneExplosions = new ArrayList<Explosion>();
			for (Explosion e : booms) {
				if (e.isDone()) {
					doneExplosions.add(e);
				}
			}
			for (Explosion e : doneExplosions) {
				booms.remove(e);
			}
			
			//destroy all dead bullets
			for (Bullet b : deadBullets) {
				if(b.getOwner() != null){
					b.getOwner().setRdyToFire(true);
				}
		
				bullets.remove(b);
			}
			
			for(theTank dt : deadTanks)
			{
				evilTanks.remove(dt);
			}
			
			if(evilTanks.isEmpty() ){
				level ++;
				evilTanks.add(new theTank(this, new Point(0,0),level));
				evilTanks.add(new theTank(this, new Point(getMaxWidth()-3,0),level));
			}
			
			paint(g);
			return true;
			
		}
		/**
		 * This is the method for the route path that the enemy tank is going to take
		 * It also has a chance of firing a bullet while its running its path
		 * @param evilTank
		 */
			public void startRoutePath(theTank evilTank){
					
				if(evilTank.getLastMove().equals("right")){
					if(evilTank.getLocation().contains(new Point(getMaxWidth()/2,getMaxHeight()-1))
					  || evilTank.getLocation().contains(new Point(getMaxWidth()-1,getMaxHeight()-1))){
						evilTank.moveUp();
					}else {
					evilTank.moveRight();
					}
				}
				else if(evilTank.getLastMove().equals("left")){
					if(evilTank.getLocation().contains(new Point(0,0))
						|| evilTank.getLocation().contains(new Point(getMaxWidth()/2 + 2 ,0))){
						evilTank.moveDown();
					} else{
						evilTank.moveLeft();				
					}
				}
				else if(evilTank.getLastMove().equals("up")){
					if(evilTank.getLocation().contains(new Point(getMaxWidth()-1,0))
							|| evilTank.getLocation().contains(new Point(getMaxWidth()/2 , 0))){
							evilTank.moveLeft();
						} else{
							evilTank.moveUp();
						}
				}
				else if(evilTank.getLastMove().equals("down")){
					if(evilTank.getLocation().contains(new Point(0,getMaxHeight()-1))
							|| evilTank.getLocation().contains(new Point(getMaxWidth()/2+2 , getMaxHeight()-1))){
							evilTank.moveRight();
						} else{
					evilTank.moveDown();
						}
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
				player.moveUp();
			} else if (key == '8') {
				player.moveDown();
			} else if (key == '4') {
				player.moveLeft();
			} else if (key == '6') {
				player.moveRight();
			} else if (key == '5'){
				if(player.getRdyToFire()){
					bullets.add(new Bullet(player.getCannonLocation(), player.getLastMove(), player));
					player.setRdyToFire(false);
				}
			}else {
				System.out.println("Invalid Key: " + key);
			}
			
		}
		
		
		////////////////////////////getter methods/////////////////////////////
		
		public int getMaxWidth() {
			return maxWidth;
		}
		
		public int getMaxHeight() {
			return maxHeight;
		}
		
		/**
		 * game testing
		 * @throws GameConfigurationException 
		 */
		public static void main(String[] args) throws GameConfigurationException {
	        tank game = new tank();
	        GameContext context = new GameContext(game);
	        context.start();
	
		}
	
	}
