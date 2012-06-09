package pl.waw.mizinski.pszt.view;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Enumeration;
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
			setExtendedState(MAXIMIZED_BOTH);
			setVisible(true);
		}
		
		
		private JMenuBar makeMenu(){
			JMenuItem domination = new JMenuItem("Dominacja");
			domination.addActionListener(new ActionListener()
			{
				@Override
				public void actionPerformed(ActionEvent e)
				{
					makeDominationDialog();
				}

			});
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
			options.add(domination);
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
			final JLabel probability = new JLabel("Prawdopodobienstwo");
			final JLabel factor = new JLabel("Wspolczynnik mutacji");
			final JTextComponent probabilityGetter = new JTextField();
			final JTextComponent factorGetter = new JTextField();
			dialog.setLayout(new GridLayout(3,1));
			JPanel p1 = new JPanel();
			p1.setLayout(new GridLayout(1,2));
			p1.add(probability);
			p1.add(probabilityGetter);
			dialog.add(p1);
			JPanel p2 = new JPanel();
			p2.setLayout(new GridLayout(1,2));
			p2.add(factor);
			p2.add(factorGetter);
			dialog.add(p2);
			
			JButton ok = new JButton("OK");
			ok.addActionListener(new ActionListener()
			{
				@Override
				public void actionPerformed(ActionEvent arg0)
				{	
					try
					{
						Double p = new Double(probabilityGetter.getText());
						Double f = new Double(factorGetter.getText());
						blockingQueue.put(new MutationUpdateEvent(p,f));
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

		private void makeDominationDialog()
		{
			final JDialog dialog = new JDialog(PsztFrame.this, "Dominacja", true);
			dialog.setSize(260,120);
			final JRadioButton domination = new JRadioButton("Pelna dominacja");
			final JRadioButton codomination = new JRadioButton("Kodominacja");
			final ButtonGroup bGroup = new ButtonGroup();
			bGroup.add(domination);
			bGroup.add(codomination);
			
			final JLabel x = new JLabel("X:");
			final JLabel y = new JLabel("Y:");
			final JTextComponent xGetter = new JTextField();
			final JTextComponent yGetter = new JTextField();
			
			ActionListener enabler = new ActionListener()
			{
				@Override
				public void actionPerformed(ActionEvent e)
				{
					x.setEnabled(true);
					y.setEnabled(true);
					xGetter.setEnabled(true);
					yGetter.setEnabled(true);
				}
			};
			ActionListener disabler = new ActionListener()
			{
				@Override
				public void actionPerformed(ActionEvent e)
				{
					x.setEnabled(false);
					y.setEnabled(false);
					xGetter.setEnabled(false);
					yGetter.setEnabled(false);
				}
			};
			disabler.actionPerformed(null);
			
			codomination.addActionListener(disabler);
			domination.addActionListener(enabler);
			
			dialog.setLayout(new GridLayout(3,1));
			
			JPanel p1 = new JPanel();
			p1.setLayout(new GridLayout(1,2));
			p1.add(codomination);
			p1.add(domination);
			dialog.add(p1);
			
			JPanel p2 = new JPanel();
			p2.setLayout(new GridLayout(1,4));
			p2.add(x);
			p2.add(xGetter);
			p2.add(y);
			p2.add(yGetter);
			dialog.add(p2);
			
			JButton ok = new JButton("OK");
			ok.addActionListener(new ActionListener()
			{
				@Override
				public void actionPerformed(ActionEvent arg0)
				{
					Enumeration<AbstractButton> en = bGroup.getElements();
					if(en.hasMoreElements()){
						try{
							if (codomination.isSelected())
							{
								blockingQueue.put(new DominationUpdateEvent());
							}
							else if (domination.isSelected())
							{
								Double domX = new Double(xGetter.getText());
								Double domY = new Double(yGetter.getText());
								blockingQueue.put(new DominationUpdateEvent(domX, domY));
							}

						}
						catch (NumberFormatException e)
						{
							cantUpdate(e.getMessage());
						}
						catch (Exception e)
						{
							e.printStackTrace();
							throw new RuntimeException(e);
						}
						dialog.dispose();
					}
				
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
