package pl.waw.mizinski.pszt.view;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.concurrent.BlockingQueue;
import javax.swing.*;
import javax.swing.text.JTextComponent;
import pl.waw.mizinski.pszt.event.*;

public class View
{
	private final BlockingQueue<PsztEvent> blockingQueue;
														
	private PsztFrame frame;
	
	public View(final BlockingQueue<PsztEvent> blockingQueue)
	{
		this.blockingQueue = blockingQueue;
		SwingUtilities.invokeLater(new Runnable()
		{
			@Override
			public void run()
			{
				frame = new PsztFrame();
			}
		});
	}
	
	public void cantUpdate(final String message)
	{
		SwingUtilities.invokeLater(new Runnable()
		{
			@Override
			public void run()
			{
				JOptionPane.showMessageDialog(null,
						"Nie mozna wprowadzic zmian z powodu:\n" + message,
						"Blad", JOptionPane.ERROR_MESSAGE);
			}
		});
	}
	
	public void write(final String string)
	{
		SwingUtilities.invokeLater(new Runnable()
		{
			@Override
			public void run()
			{
				frame.write(string);
			}
		});
	}
	
	class PsztFrame extends JFrame
	{
		private static final long serialVersionUID = 1L;
		final JTextArea tekstArea = new JTextArea();
		final JButton start = new JButton("START");
		final JButton stop = new JButton("STOP");
		final JButton clear = new JButton("CZYSC"); 
		
		PsztFrame()
		{
			super("Podstawy Sztucznej Inteligencji");
			try{
				 UIManager.setLookAndFeel("com.sun.java.swing.plaf.nimbus.NimbusLookAndFeel");
			}catch(Exception e){
				e.printStackTrace();
				throw new RuntimeException(e);
			}
			setJMenuBar(makeMenu());		
			start.addActionListener(new ActionListener()
			{
				@Override
				public void actionPerformed(ActionEvent arg0)
				{
					try
					{
						blockingQueue.put(new StartEvent());
					}
					catch (InterruptedException e)
					{
						e.printStackTrace();
						throw(new RuntimeException(e));
					}
					stop.setEnabled(true);
					start.setEnabled(false);
				}
			});
			stop.setEnabled(false);
			stop.addActionListener(new ActionListener()
			{
				@Override
				public void actionPerformed(ActionEvent arg0)
				{
					try
					{
						blockingQueue.put(new StopEvent());
					}
					catch (InterruptedException e)
					{
						e.printStackTrace();
						throw(new RuntimeException(e));
					}
					stop.setEnabled(false);
					start.setEnabled(true);
				}
			});
			clear.addActionListener(new ActionListener()
			{
				@Override
				public void actionPerformed(ActionEvent arg0)
				{
					tekstArea.setText("");
				}
			});
			add(BorderLayout.CENTER,new JScrollPane(tekstArea));
			JPanel panel = new JPanel();
			panel.setLayout(new FlowLayout());
			panel.add(start);
			panel.add(stop);
			panel.add(clear);
			add(BorderLayout.SOUTH,panel);
			setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
			setSize(640, 480);
			//setExtendedState(MAXIMIZED_BOTH);
			setVisible(true);
		}
		
		
		private JMenuBar makeMenu(){
			JMenuItem mutation = new JMenuItem("Mutacje");
			mutation.addActionListener(new ActionListener()
			{
				@Override
				public void actionPerformed(ActionEvent e)
				{
					makeMutationDialog();
				}
			});
			
			JMenuItem size = new JMenuItem("Rozmiar populacji");
			size.addActionListener(new ActionListener()
			{
				@Override
				public void actionPerformed(ActionEvent e)
				{
					makePopulationSizeDialog();
				}
			});
			
			JMenu options = new JMenu("Opcje");
			options.add(mutation);
			options.add(size);
			JMenuBar menu = new JMenuBar();
			menu.add(options);
			return menu;
		}


		private void makePopulationSizeDialog()
		{
			final JDialog dialog = new JDialog(PsztFrame.this, "Rozmiar populacji", true);
			dialog.setSize(200,120);
			final JLabel size = new JLabel("Rozmiar populacji:");
			final JTextComponent sizeGetter = new JTextField();
			dialog.setLayout(new GridLayout(3,1));
			dialog.add(size);
			dialog.add(sizeGetter);
			JButton ok = new JButton("OK");
			ok.addActionListener(new ActionListener()
			{
				@Override
				public void actionPerformed(ActionEvent arg0)
				{	
					try
					{
						Integer s = new Integer(sizeGetter.getText());
						blockingQueue.put(new SizeUpdateEvent(s));
					}
					catch (NumberFormatException e)
					{
						cantUpdate(e.getMessage());
					}
					catch (InterruptedException e)
					{
						e.printStackTrace();
						throw new RuntimeException(e);
					}
					dialog.dispose();
				}
			});
			JPanel okPanel = new JPanel();
			okPanel.setLayout(new FlowLayout());
			okPanel.add(ok);
			dialog.add(okPanel);
			dialog.setVisible(true);
		}


		private void makeMutationDialog()
		{
			final JDialog dialog = new JDialog(PsztFrame.this, "Mutacje", true);
			dialog.setSize(300,120);
			final JLabel probability = new JLabel("Prawdopodobienstwo mutacji");
			final JTextComponent probabilityGetter = new JTextField();
			dialog.setLayout(new GridLayout(3,1));
			dialog.add(probability);
			dialog.add(probabilityGetter);
			JButton ok = new JButton("OK");
			ok.addActionListener(new ActionListener()
			{
				@Override
				public void actionPerformed(ActionEvent arg0)
				{	
					try
					{
						Double p = new Double(probabilityGetter.getText());
						blockingQueue.put(new MutationUpdateEvent(p));
					}
					catch (NumberFormatException e)
					{
						cantUpdate(e.getMessage());
					}
					catch (InterruptedException e)
					{
						e.printStackTrace();
						throw new RuntimeException(e);
					}
					dialog.dispose();
				}
			});
			JPanel okPanel = new JPanel();
			okPanel.setLayout(new FlowLayout());
			okPanel.add(ok);
			dialog.add(okPanel);
			dialog.setVisible(true);
		}


		void write(String string){
			tekstArea.append(string + '\n');
		}
	}
}
