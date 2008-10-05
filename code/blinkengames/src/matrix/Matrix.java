package matrix;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.util.ArrayList;

import de.blinkenlights.game.BlinkenGame;
import de.blinkenlights.game.FrameInfo;
import de.blinkenlights.game.GameConfigurationException;
import de.blinkenlights.game.GameContext;

public class Matrix implements BlinkenGame{
	/**
	 * The max width of the screen.
	 */
	public static final int MAX_WIDTH = 96;
	
	/**
	 * The max height of the screen.
	 */
	public static final int MAX_HEIGHT = 32;
	
	/**
	 * Number of milliseconds that will tick before the animation updates itself.
	 */
	public static final int ANIM_TICK = 80;
	
	private GameContext context;
	
	/**
	 * The vertical lines that the animation consists of. 
	 */
	private ArrayList<Line> lines;
	
	public void paint(Graphics g) {
		g.clearRect(0, 0, MAX_WIDTH, MAX_HEIGHT);
		g.setColor(Color.WHITE);
		
		int size = lines.size();
		for (int i = 0; i < size; i++) {
			
			Line line = lines.get(i);
			
			if (line.isEmpty()) {
				lines.remove(line);
				lines.trimToSize();
				size--;
				continue;
			}
			line.paint(g);
		}
	}
	
	public static void main(String[] args) throws GameConfigurationException {
        Matrix game = new Matrix();
        GameContext context = new GameContext(game);
        context.start();
    }

	public void gameEnding(Graphics2D arg0) {
		//do nothing
	}

	public void gameStarting(GameContext gameContext) {
		context = gameContext;
        context.setFramesPerSecond(12.5);
        
        lines = new ArrayList<Line>();
		for(int i = 1; i < 10; i++) {
			Line temp = new Line();
			lines.add(temp);
		}
	}

	public boolean nextFrame(Graphics2D g, FrameInfo frameInfo) {
		Line temp = new Line();
		lines.add(temp);
		paint(g);
		
        // quit after 3 minutes so a loop can be recorded
        return frameInfo.getWhen() < 180000;
	}
}
