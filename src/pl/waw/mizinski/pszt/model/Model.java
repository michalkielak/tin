package pl.waw.mizinski.pszt.model;

import java.util.concurrent.BlockingQueue;

import pl.waw.mizinski.pszt.event.PsztEvent;
import pl.waw.mizinski.pszt.event.WriteEvent;
import pl.waw.mizinski.pszt.model.population.Population;

public class Model
{
	private int populationSize = 1000;
	private double mutationProbability = 0.01;
	private double mutationFactor = 0.1;
	private boolean codomination = true;
	private double domX = 1;
	private double domY = 1;
	
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
				+ "Prawdopodobienstwo mutacji: " + mutationProbability + "\n"
				+ "Wspolczynnik mutacji: "	+ mutationFactor + "\n");
		if(codomination)
		{
			info += "Kodominacja\n";
		}
		else
		{
			info += "Pelna dominacja wokol punktu ( " + domX + " , " + domY + " )\n";
		}
		
		try
		{
			blockingQueue.put(new WriteEvent(info));
		}
		catch (InterruptedException e)
		{
			e.printStackTrace();
			throw new RuntimeException(e);
		}
		
		population = new Thread(new Population(populationSize,
				mutationProbability, mutationFactor, codomination, domX, domY,
				blockingQueue));
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

	public void setMutationFactor(double mutationFactor)
	{
		this.mutationFactor = mutationFactor;
	}

	public void setCodomination(boolean codomination)
	{
		this.codomination = codomination;
	}

	public void setDomX(double domX)
	{
		this.domX = domX;
	}

	public void setDomY(double domY)
	{
		this.domY = domY;
	}
	
	public static boolean checkPopulationSize(int populationSize)
	{
		if(populationSize >= 100 && populationSize <=20000)
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

	public static boolean checkMutationFactor(double mutationFactor)
	{
		if(mutationFactor >= 0 && mutationFactor <= 4 )
		{
			return true;
		}
		return false;
	}
	
	public static boolean checkDomX(double domX)
	{
		if(domX >= -2 && domX <= 2)
		{
			return true;
		}
		return false;
	}

	public static boolean checkDomY(double domY)
	{
		if(domY >= -2 && domY <= 2)
		{
			return true;
		}
		return false;
	}
	
}
