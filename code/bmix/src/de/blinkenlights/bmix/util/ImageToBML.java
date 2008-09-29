package de.blinkenlights.bmix.util;

import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.image.BufferedImage;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.OutputStream;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.prefs.BackingStoreException;
import java.util.prefs.Preferences;

import javax.imageio.ImageIO;
import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JProgressBar;
import javax.swing.JSpinner;
import javax.swing.JTextField;
import javax.swing.SpinnerNumberModel;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

import org.jdesktop.swingworker.SwingWorker;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.factories.ButtonBarFactory;
import com.jgoodies.forms.layout.FormLayout;
import com.l2fprod.common.swing.JDirectoryChooser;

import de.blinkenlights.bmix.movie.BMLOutputStream;

public class ImageToBML {

	JFrame mainFrame = new JFrame();
	private ConvertTask task;
	public static final String version = "Version $Id$";

	JTextField xSizeField = new JTextField();
	JTextField ySizeField = new JTextField();
	JTextField pngDir = new JTextField();
	JTextField outFile = new JTextField();

	// metadata
	JTextField titleField = new JTextField();
	JTextField descriptionField = new JTextField();
	JTextField authorField = new JTextField();
	JTextField emailField = new JTextField();
	JTextField urlField = new JTextField();
	
	JButton pngDirChoose = new JButton("...");
	JButton outFileChoose = new JButton("...");
	JButton startButton = new JButton("Start");
	JButton cancelButton = new JButton("Cancel");
	JSpinner frameRateField = new JSpinner(new SpinnerNumberModel(1,1,100,1));
	JLabel statusLine = new JLabel(version);
	JProgressBar progress = new JProgressBar();

	private Preferences prefs;

	InputVerifier inputVerifier = new InputVerifier();
	
	
	
    public void createAndShowGUI() {
    	
    	prefs = Preferences.userNodeForPackage(ImageToBML.class);
    	
        //Create and set up the main window.
        mainFrame.setTitle("Image Sequence to BML");
   
	    mainFrame.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
	    
	    mainFrame.addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent e) {
                prefs.putInt("frameX", mainFrame.getX());
                prefs.putInt("frameY", mainFrame.getY());
                prefs.putInt("frameWidth", mainFrame.getWidth());
                prefs.putInt("frameHeight", mainFrame.getHeight());
                try {
                    prefs.flush();
                } catch (BackingStoreException e1) {
                    e1.printStackTrace();
                }
                System.exit(0);
            }
	    });

        pngDirChoose.addActionListener(new ActionListener() {
        	public void actionPerformed(ActionEvent e) {
        		JDirectoryChooser chooser = new JDirectoryChooser();
        		int choice = chooser.showOpenDialog(mainFrame);
        		if(choice == JDirectoryChooser.CANCEL_OPTION) {
        			System.out.println("User Canceled");					
        		}
        		else {
        			pngDir.setText(chooser.getSelectedFile().getAbsolutePath());
        		}
        	}	
        });
        
        outFileChoose.addActionListener(new ActionListener() {
        	public void actionPerformed(ActionEvent e) {
        		JFileChooser chooser = new JFileChooser();
        		int choice = chooser.showSaveDialog(mainFrame);
        		if(choice == JDirectoryChooser.CANCEL_OPTION) {
        			System.out.println("User Canceled");					
        		}
        		else {
        			outFile.setText(chooser.getSelectedFile().getAbsolutePath());
        		}
        	}	
        });
        
        startButton.addActionListener(new ActionListener() {

			public void actionPerformed(ActionEvent e) {
				File out = new File(outFile.getText());
				if (out.exists()) {
					int ans = JOptionPane.showConfirmDialog(null, "" + out.getName() + " exists. Overwrite?", "Save Over Existing File", JOptionPane.OK_CANCEL_OPTION, JOptionPane.QUESTION_MESSAGE);
					if (ans == JOptionPane.CANCEL_OPTION) {
						return;
					}
				}
        		task = new ConvertTask(new File(pngDir.getText()), out,new Dimension(Integer.parseInt(xSizeField.getText()),Integer.parseInt(ySizeField.getText())),4);
        		task.addPropertyChangeListener(new PropertyChangeListener() {

					public void propertyChange(PropertyChangeEvent event) {
						if (event.getPropertyName().equals("progress")) {
							progress.setValue((Integer)event.getNewValue());
						}
					}
        			
        		});
        		task.execute();
        	}
        });

        
        cancelButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				if (task != null) {
					task.cancel(true);
				}
        	}
        });

        
        FormLayout mainLayout = new FormLayout( 
        		"pref, 4dlu, 120dlu:grow, 4dlu, min",
        		"");
  
        pngDir.setText(prefs.get("pngDir", System.getProperty("user.home")));
        pngDir.getDocument().addDocumentListener(inputVerifier);
        
        outFile.setText(prefs.get("outFile", System.getProperty("user.home")+System.getProperty("file.separator")+"movie.bml"));
        outFile.getDocument().addDocumentListener(inputVerifier);
        
        xSizeField.setText(prefs.get("xSize", "18"));
        xSizeField.getDocument().addDocumentListener(inputVerifier);
        
        ySizeField.setText(prefs.get("ySize", "8"));
        ySizeField.getDocument().addDocumentListener(inputVerifier);
        
        frameRateField.setValue(prefs.getInt("fps",15));
        frameRateField.getModel().addChangeListener(inputVerifier);
        
//        titleField.setText(prefs.get("title",""));
//        titleField.getDocument().addDocumentListener(inputVerifier);
        
//        descriptionField.setText(prefs.get("description",""));
//        descriptionField.getDocument().addDocumentListener(inputVerifier);
        
        authorField.setText(prefs.get("author",System.getProperty("user.name")));
        authorField.getDocument().addDocumentListener(inputVerifier);
        
        emailField.setText(prefs.get("email",""));
        emailField.getDocument().addDocumentListener(inputVerifier);

        urlField.setText(prefs.get("url",""));
        urlField.getDocument().addDocumentListener(inputVerifier);
        
        cancelButton.setEnabled(false);
        progress.setStringPainted(true);
        
        DefaultFormBuilder mainBuilder = new DefaultFormBuilder(mainLayout); 
        mainBuilder.setDefaultDialogBorder();
        
        mainBuilder.appendSeparator("Movie Parameters");
        mainBuilder.nextLine();
        
        mainBuilder.append("Source Directory",pngDir,pngDirChoose);
        mainBuilder.nextLine();
        mainBuilder.append("Animation Width",xSizeField);
        mainBuilder.nextLine();
        mainBuilder.append("Animation Height",ySizeField);
        mainBuilder.nextLine();
        mainBuilder.append("Animation FPS",frameRateField);
        mainBuilder.nextLine();
        mainBuilder.append("Output File",outFile, outFileChoose);
        mainBuilder.nextLine();

        mainBuilder.appendUnrelatedComponentsGapRow();
        mainBuilder.nextLine();

        mainBuilder.appendSeparator("Movie Information (optional)");
        mainBuilder.nextLine();
        
        mainBuilder.append("Title", titleField);
        mainBuilder.nextLine();
        mainBuilder.append("Description", descriptionField);
        mainBuilder.nextLine();
        mainBuilder.append("Author",authorField);
        mainBuilder.nextLine();
        mainBuilder.append("Email",emailField);
        mainBuilder.nextLine();
        mainBuilder.append("URL", urlField);
        mainBuilder.nextLine();

       
        

        mainBuilder.appendUnrelatedComponentsGapRow();
        mainBuilder.nextLine();

        
        mainBuilder.append(ButtonBarFactory.buildOKCancelBar(startButton, cancelButton),5);
        mainBuilder.nextLine();
        
        mainBuilder.appendUnrelatedComponentsGapRow();
        mainBuilder.nextLine();
        
        mainBuilder.append(progress,5);
        mainBuilder.nextLine();
        
        mainBuilder.append(statusLine,5);
        mainBuilder.nextLine();
        
	    mainFrame.getContentPane().add(mainBuilder.getPanel());
        mainFrame.pack();
	    
		int x = prefs.getInt("frameX", 20);
		int y = prefs.getInt("frameY", 20);
		int width = prefs.getInt("frameWidth", mainFrame.getPreferredSize().width);
		int height = prefs.getInt("frameHeight", mainFrame.getPreferredSize().height);
		mainFrame.setBounds(x, y, width, height);

        mainFrame.setVisible(true);
        
        inputVerifier.verifyAndSave();
    }
      
    private class InputVerifier implements DocumentListener, ChangeListener {

		public void changedUpdate(DocumentEvent arg0) {
			verifyAndSave();
		}

		public void insertUpdate(DocumentEvent arg0) {
			verifyAndSave();
		}

		public void removeUpdate(DocumentEvent arg0) {
			verifyAndSave();
		}

		public void stateChanged(ChangeEvent arg0) {
			verifyAndSave();
		}
    	
		public void verifyAndSave() {
			startButton.setEnabled(false);
			prefs.put("pngDir", pngDir.getText());
			prefs.put("outFile", outFile.getText());
			prefs.put("xSize", xSizeField.getText());
			prefs.put("ySize", ySizeField.getText());
			prefs.putInt("fps", ((Number)frameRateField.getValue()).intValue());
			
			prefs.put("author", authorField.getText());
			prefs.put("email", emailField.getText());
			prefs.put("url", urlField.getText());
			
			if (!new File(pngDir.getText()).isDirectory()) {
				statusLine.setText("Source Directory isn't a directory.");
				return;
			}
			
			File outFileParent = new File(outFile.getText()).getAbsoluteFile().getParentFile();
			File out = new File(outFile.getText());
			
			if (outFileParent == null || out.isDirectory() || !outFileParent.exists()) {
				statusLine.setText("Output directory is invalid");
				return;
			}
			
			
			try {
				int xSize = Integer.parseInt(xSizeField.getText());
				if (xSize < 1) {
					statusLine.setText("Width must be positive.");
					return;
				}
			} catch (NumberFormatException e) {
				statusLine.setText("Width must be an integer.");
				return;
			}

			try {
				int ySize = Integer.parseInt(ySizeField.getText());
				if (ySize < 1) {
					statusLine.setText("Height must be positive.");
					return;
				}
			} catch (NumberFormatException e) {
				statusLine.setText("Height must be an integer.");
				return;
			}		
			
			statusLine.setText(version);
			startButton.setEnabled(true);
		}

    }
		
    private static class ImageFileNameFilter implements FilenameFilter {

		public boolean accept(File dir, String name) {
			File file = new File(dir,name);
			if (name.startsWith(".") || file.isHidden() || file.isDirectory()) {
				return false;
			}
			return true;
		}
    	
    }
    private class ConvertTask extends SwingWorker<Object, Object>{

    	private final File pngDir2;
		private final File output;
		private final Dimension size;
		private final int bpp;
		private String currentFileName;
		private OutputStream outputStream;
		private int filesProcessed;
		
		protected ConvertTask(File pngDir, File output, Dimension size, int bpp) {
			pngDir2 = pngDir;
			this.output = output;
			this.size = size;
			this.bpp = bpp;
			startButton.setEnabled(false);
			cancelButton.setEnabled(true);
			progress.setValue(0);
			//progress.setVisible(true);
    	}
		
		@Override
		protected Object doInBackground() throws Exception {
    		outputStream = new FileOutputStream(output);
			File[] files = pngDir2.listFiles();
    		Arrays.sort(files);
    		
    		Map<String,String> headerData = new HashMap<String,String>();
    		headerData.put("author",authorField.getText());
    		headerData.put("description",descriptionField.getText());
    		headerData.put("title",titleField.getText());
    		headerData.put("email",emailField.getText());
    		headerData.put("url", urlField.getText());
    		
    		BMLOutputStream bmlOut = new BMLOutputStream(outputStream,((Number)frameRateField.getValue()).intValue(),size, bpp, headerData);
    		
    		File[] fileList = pngDir2.listFiles(new ImageFileNameFilter());
    		filesProcessed = 0;
			for (File file : fileList) {
    			currentFileName = file.getName();
    			statusLine.setText("Processing: "+currentFileName);
    			if (isCancelled()) {
    				return null;
    			}
    			BufferedImage image=(BufferedImage)ImageIO.read(file);
    			if (image != null) {
    				bmlOut.writeFrame(image);
    			} else {
    				System.err.println("error processing "+file);
    				throw new Exception("<html>ImageIO is unable to convert this file.<br>Processing aborted.");
    			}
    			filesProcessed++;
    			int progress = (int)(filesProcessed / (float)fileList.length * 100);
    			setProgress(progress);
    			
    		}
    		bmlOut.close();
			return null;
		}

		@Override
		protected void done() {
			super.done();
			startButton.setEnabled(true);
			cancelButton.setEnabled(false);
			statusLine.setText(version);
			progress.setValue(0);
			try {
				get();
				JOptionPane.showMessageDialog(mainFrame, "<html>Done!<br>"+filesProcessed+" frames processed.");
			} catch (Exception e) {
				Throwable cause = e;
				while (cause.getCause() != null) {
					cause = cause.getCause();
				}
				if (currentFileName != null) {
					JOptionPane.showMessageDialog(mainFrame, "<html>Error processing <b>"+currentFileName+"</b>:<br>"+cause.getMessage());
				} else {
					JOptionPane.showMessageDialog(mainFrame, "<html>Error: "+e.getMessage());
				}
				if (outputStream != null) {
					try {
						outputStream.close();
					} catch (IOException e1) {
						//no big deal...
					}
				}
				// remove broken output file
				output.delete();
				e.printStackTrace();
			}

		}
    }
        
	/**
	 * @param args
	 */
	public static void main(String[] args) throws IOException {
		
		final ImageToBML main = new ImageToBML();
        main.createAndShowGUI();
        // wait for swing to exit...
        
		/*
		if (args.length != 4) {
			System.out.println("usage: <png directory> <x> <y> <output file>");
			return;
		}

		*/
		
	}
	
		
}
