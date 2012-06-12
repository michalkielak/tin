package pl.waw.mizinski.pszt.model;

import java.util.concurrent.BlockingQueue;

import pl.waw.mizinski.pszt.event.PsztEvent;
import pl.waw.mizinski.pszt.event.WriteEvent;
import pl.waw.mizinski.pszt.model.population.Population;

public class Model
{
	private int populationSize = 1000;
	private double mutationProbability = 0.01;

	
	private final BlockingQueue<PsztEvent> blockingQueue;
	private Thread population = null;
	
	public Model(BlockingQueue<PsztEvent> blockingQueue)
	{
		super();
		this.blockingQueue = blockingQueue;
	}

	public void start()
	{
		String info = new String("Popucja o wielkosci: " + populationSize + " osobnikow\n"
				+ "Prawdopodobienstwo mutacji: " + mutationProbability + "\n");
	
		try
		{
			blockingQueue.put(new WriteEvent(info));
		}
		catch (InterruptedException e)
		{
			e.printStackTrace();
			throw new RuntimeException(e);
		}
		
		population = new Thread (new Population(populationSize, mutationProbability, blockingQueue));
		population.start();
	}
	
	public void stop()
	{
		population.interrupt();
	}
	
	public void setPopulationSize(int populationSize)
	{
		this.populationSize = populationSize;
	}

	public void setMutationProbability(double mutationProbability)
	{
		this.mutationProbability = mutationProbability;
	}

	
	
	public static boolean checkPopulationSize(int populationSize)
	{
		if(populationSize >= 10 && populationSize <=20000)
		{
			return true;
		}
		return false;
	}
	
	public static boolean checkMutationProbability(double mutationProbability)
	{
		if(mutationProbability >= 0 && mutationProbability <=1 )
		{
			return true;
		}
		return false;
	}
	
}
