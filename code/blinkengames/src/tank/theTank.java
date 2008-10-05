package tank;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Point;
import java.util.ArrayList;
import java.util.List;

	public class theTank {
		
		private Point location;
		private List<Point> tankLocation;
		private tank game;
		private String moved;
		private boolean rdyToFire;
		private int hp;
		private Color colour;
		
		/**
		 * The starting location of the tank in the center of the screen
		 */
		theTank(tank game) {
			this.game = game;
			tankLocation = new ArrayList<Point>();
			location = new Point(game.getMaxWidth()/2, game.getMaxHeight()/2);
			moved = "left";
			rdyToFire = true;
		}
		/**
		 * overLoading method for the computer Tanks
		 * @param game
		 * @param spawn
		 * @param hp
		 */
		theTank(tank game,Point spawn, int hp) {
			this.game = game;
			tankLocation = new ArrayList<Point>();
			location = spawn;
			this.hp = hp;
			colour = new Color(0xaaaaaa);
			if(location.x == 0 && location.y == 0){
				moved = "down";
			}
			else if(location.x == 0 && location.y == game.getMaxHeight()-3){
				moved = "right";
			}
			else if(location.x == game.getMaxWidth()-3 && location.y == game.getMaxHeight()-3){
				moved = "up";
			}
			else if(location.x == game.getMaxWidth()-3 && location.y == 0){
				moved = "left";
			}
			else {
				System.out.println("tank incorrect spawn location");
			}
			updateLocation();
			rdyToFire = true;
		}
		
		public void moveLeft(){
			
			if(location.x > 0 ){
				
				location.x --;
				updateLocation();
				if(!checkForWalls()){
					location.x++;
				}
				moved = "left";
			}
		}
		public void moveRight(){
			if(location.x < game.getMaxWidth()-3){
				location.x ++;
				updateLocation();
				if(!checkForWalls()){
					location.x--;
				}
				moved = "right";
			}
		}
		public void moveUp(){
			if(location.y > 0 ){
				location.y --;
				updateLocation();
				if(!checkForWalls()){
					location.y++;
				}
				moved = "up";
			}
		}
		public void moveDown(){
			if(location.y < game.getMaxHeight()-3){
				location.y ++;
				updateLocation();
				if(!checkForWalls()){
					location.y--;
				}
				moved = "down";
			}
			
		}
		public List<Point> getLocation(){	
			return tankLocation;
		}
		public String getLastMove(){
			return moved;
		}
		public void setMoved(String move){
			moved = move; 
		}
		public void setRdyToFire(boolean isrdy) {
			rdyToFire = isrdy;
		}
		public boolean getRdyToFire(){
			return rdyToFire;
		}
		public int getHp(){
			return hp;
		}
		public void damged(){
			hp--;
		}
		
		public boolean checkForWalls(){
			for(Point p : tankLocation ){
				if(game.getWalls().contains(p)){
					return false;
				}	
			}
			return true;
			
		}
		public Point getCannonLocation(){
			if(moved.equals("left")){
				return new Point(location.x-1, location.y+1);
			}
			else if(moved.equals("right")) {
				return new Point(location.x+3, location.y+1);
			}
			else if(moved.equals("up")) {
				return new Point(location.x+1, location.y-1);
			}
			else if(moved.equals("down")) {
				return new Point(location.x+1, location.y+3);
			}
			return  new Point(0,0);
		}
		/**
		 * updates the location of the tank 
		 */
		private void updateLocation(){
			
			tankLocation.clear();
			tankLocation.add(new Point(location.x,location.y));
			tankLocation.add(new Point(location.x+1,location.y));
			tankLocation.add(new Point(location.x+2,location.y));
			tankLocation.add(new Point(location.x+2,location.y+1));
			tankLocation.add(new Point(location.x+2,location.y+2));
			tankLocation.add(new Point(location.x+1,location.y+2));
			tankLocation.add(new Point(location.x,location.y+2));
			tankLocation.add(new Point(location.x,location.y+1));
			tankLocation.add(new Point(location.x+1,location.y+1));
		}
		
		
		public void paint(Graphics g) {
			if(colour != null){
				g.setColor(colour);
			}
			g.drawRect(location.x, location.y, 2, 2);
			if(moved.equals("left")){
				g.drawLine(location.x-1, location.y+1, location.x-1, location.y+1);
			}
			else if(moved.equals("right")) {
				g.drawLine(location.x+3, location.y+1, location.x+3, location.y+1);
			}
			else if(moved.equals("up")) {
				g.drawLine(location.x+1, location.y-1, location.x+1, location.y-1);	
			}
			else if(moved.equals("down")) {
			g.drawLine(location.x+1, location.y+3, location.x+1, location.y+3);
		}
		else {
			System.out.println("impossible movement");
		}
		
		

	}
		
	

}
