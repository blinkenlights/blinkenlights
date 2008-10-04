package util;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Properties;

import de.blinkenlights.game.GameConfigurationException;
import de.blinkenlights.game.GameContext;

public class GameLoader {
	
	private final List<GameContext> gameContexts = new ArrayList<GameContext>();
	
	public void loadGames() throws IOException, GameConfigurationException {
		Properties config = new Properties();
		FileInputStream fileInputStream = new FileInputStream("gameLoader.properties");
		config.load(fileInputStream);
		fileInputStream.close();
		
		int i = 0;
		while (config.getProperty("gameProperties."+i) != null) {
			File gamePropertyFile = new File(config.getProperty("gameProperties."+i));
			Properties gameConfig = new Properties();
			FileInputStream gamePropertiesInputStream = new FileInputStream(gamePropertyFile);
			gameConfig.load(gamePropertiesInputStream);
			gamePropertiesInputStream.close();
			
			GameContext gc = new GameContext(gameConfig);
			new Thread(gc).start();
			gameContexts.add(gc);
			
			i++;
		}
		
	}

	/**
	 * Loads all games described in the gameLoader.properties file, which must
	 * be in the current working directory.
	 * 
	 * @param args
	 *            ignored
	 * @throws IOException
	 *             If the gameLoader file or any of the properties files it
	 *             references can't be read
	 * @throws GameConfigurationException
	 *             If the contents of any of the properties files are invalid or
	 *             missing.
	 */
	public static void main(String[] args) throws IOException, GameConfigurationException {
		GameLoader loader = new GameLoader();
		loader.loadGames();
	}
}
