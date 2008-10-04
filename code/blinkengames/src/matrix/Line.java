package matrix;

import java.awt.Graphics;
import java.util.ArrayList;

public class Line {
	/**
	 * A line consists of an array of letters which cascade vertically downwards.
	 */
	private ArrayList<Letter> line;
	
	/**
	 * Tracks each line to determine if it is being deleted or not.
	 */
	private boolean isDone = false;
	
	public Line(Graphics g) {
		line = new ArrayList<Letter>();
		line.add(new Letter());
		paint(g);
	}
	
	public Line() {
		line = new ArrayList<Letter>();
		line.add(new Letter());
	}
	
	public int size() {
		return line.size();
	}
	
	public boolean isEmpty() {
		return line.isEmpty();
	}
	
	private Letter get(int o) {
		return line.get(o);
	}
	
	private void remove(Object f) {
		line.remove(f);
	}
	
	private void trimToSize() {
		line.trimToSize();
	}
	
	public void add(Letter f) {
		line.add(f);
	}
	
	/**
	 * Loop through the line. If a single  letter within it is done, begin to delete the 
	 * line. Otherwise, add another 3 letter to the line.
	 */
	public void paint(Graphics g) {
		int innersize = size();
		
		for(int j = 0; j < innersize; j++) {
			Letter f = get(j);
			
			if (f.isDone()) {
				isDone = true;
				remove(f);
				trimToSize();
				innersize--;
			} else{
				f.paint(g, f.getLetterPosition());
				if (j == innersize-1 && j < 20  && !isDone) {					
					for (int k = 0; k < 3; k++) {
						Letter temp = new Letter((int)f.getLetterPosition().getX(), (int)f.getLetterPosition().getY()+ k, k*2);
						add(temp);
						temp.paint(g, temp.getLetterPosition());
					}
				}
			}	
		}
	}
}
