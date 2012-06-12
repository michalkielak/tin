package pl.waw.mizinski.pszt.controller;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.BlockingQueue;

import pl.waw.mizinski.pszt.event.*;
import pl.waw.mizinski.pszt.model.Model;
import pl.waw.mizinski.pszt.view.View;



public class Controller
{
	private final View view;	
	private final Model model;
	private final BlockingQueue<PsztEvent> blockingQueue;
	private final Map<Class<? extends PsztEvent>, PsztAction> eventActionMap;
	public Controller(View view, Model model,
			BlockingQueue<PsztEvent> blockingQueue)
	{
		super();
		this.view = view;
		this.model = model;
		this.blockingQueue = blockingQueue;
		eventActionMap = new HashMap<Class<? extends PsztEvent>, PsztAction>();
		fillEventActionMap();
	}
	
	private void fillEventActionMap()
	{
		
		eventActionMap.put(MutationUpdateEvent.class, new PsztAction()
		{	
			@Override
			public void go(PsztEvent e)
			{
				MutationUpdateEvent event = (MutationUpdateEvent) e;

				if (Model.checkMutationProbability(event.probability) )
				{
					model.setMutationProbability(event.probability);
				}
				else
				{
					view.cantUpdate("Prawdopodobienstwo musi nalezec do przedzialu <0,1>");
				}
			}
			
		});
		
		eventActionMap.put(SizeUpdateEvent.class, new PsztAction()
		{
			@Override
			public void go(PsztEvent e)
			{
				SizeUpdateEvent event = (SizeUpdateEvent) e;
				if (Model.checkPopulationSize(event.size))
				{
					model.setPopulationSize(event.size);
				}
				else
				{
					view.cantUpdate("Licznosc populacji musi miescic sie w przedziale <10,20 000>");
				}
			
			}
		});
		
		eventActionMap.put(StartEvent.class, new PsztAction()
		{
			@Override
			public void go(PsztEvent e)
			{
				model.start();
			}
		});
		
		eventActionMap.put(StopEvent.class, new PsztAction()
		{
			@Override
			public void go(PsztEvent e)
			{
				model.stop();
			}
		});
		
		eventActionMap.put(WriteEvent.class, new PsztAction()
		{		
			@Override
			public void go(PsztEvent e)
			{
				WriteEvent event = (WriteEvent) e;
				view.write(event.message);
			}
		});
	}
	
	public void work()
	{
		while(true)
		{
			try
			{
				PsztEvent event= blockingQueue.take();
				PsztAction action = eventActionMap.get(event.getClass());
				action.go(event);
			}
			catch (InterruptedException e)
			{
				e.printStackTrace();
				throw new RuntimeException(e);
			}
		}
	}
}
