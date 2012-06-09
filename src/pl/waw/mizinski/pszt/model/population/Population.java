package pl.waw.mizinski.pszt.model.population;

import java.util.Arrays;
import java.util.Random;
import java.util.concurrent.BlockingQueue;

import pl.waw.mizinski.pszt.event.PsztEvent;
import pl.waw.mizinski.pszt.event.WriteEvent;

public class Population implements Runnable
{
	private final int size;
	private final double mutationProbability;
	private final double mutationFactor;
	private final boolean codomination;
	private final double domX;
	private final double domY;
	
	private final BlockingQueue<PsztEvent> blockingQueue;
	
	private static Random random = new Random();
	
	public Population(int size, double mutationProbability, double mutationFactor,
			boolean codomination, double domX, double domY, BlockingQueue<PsztEvent> blockingQueue)
	{
		super();
		this.size = size;
		this.mutationProbability = mutationProbability;
		this.mutationFactor = mutationFactor;
		this.codomination = codomination;
		this.domX = domX;
		this.domY = domY;
		this.blockingQueue = blockingQueue;
	}


	@Override
	public void run()
	{	
		Individual[] individuals = new Individual[size];
		for(int i=0; i<size; ++i)
		{
			individuals[i] = new Individual();
		}
		Arrays.sort(individuals);
		Individual temp = individuals[0];
		while(!Thread.interrupted())
		{
			if(temp!=individuals[0])
			{
				temp=individuals[0];
				try
				{
					blockingQueue.put(new WriteEvent(temp.toString()));
				}
				catch (InterruptedException e)
				{
					e.printStackTrace();
					throw new RuntimeException(e);
				}
			}
			Individual[] children = new Individual[size];
			for(int i=0; i<size; ++i)
			{
				children[i] =  getChild(individuals[random.nextInt(size)],individuals[random.nextInt(size)]);
			}
			Individual[] all = new Individual [2*size];
			System.arraycopy(individuals, 0, all, 0, size);
			System.arraycopy(children, 0, all, size, size);
			Arrays.sort(all);
			individuals = Arrays.copyOf(all,size);
		}
	}

	private Individual getChild(Individual a, Individual b)
	{
		Individual Individual =  new Individual(a.getRandomX(), b.getRandomX(), a.getRandomY(), b.getRandomY());
		Individual.mutate();
		return Individual;
	}
	
	private class Individual implements Comparable<Individual>
	{
		double x0,x1,y0,y1;
	
		
		public Individual(double x0, double x1, double y0, double y1)
		{
			super();
			this.x0 = x0;
			this.x1 = x1;
			this.y0 = y0;
			this.y1 = y1;
		}

		public Individual()
		{
			x0 = random.nextDouble()*4.0 -2.0;
			x1 = random.nextDouble()*4.0 -2.0;
			y0 = random.nextDouble()*4.0 -2.0;
			y1 = random.nextDouble()*4.0 -2.0;
		}
		
		public Individual(Individual individual)
		{
			super();
			this.x0 = individual.x0;
			this.x1 = individual.x1;
			this.y0 = individual.y0;
			this.y1 = individual.y1;
		}
		

		public double getFenotyp(){
			return codomination ? getCodomFenotyp() : getDomFenotyp();
		}
		
		
		private double getCodomFenotyp()
		{
			double x = (x0+x1)/2;
			double y = (y0+y1)/2;
			
			return (1.0-x)*(1.0-x) + 100.0*(y - x*x)*(y - x*x);
		}
		
		private double getDomFenotyp()
		{
			double x = Math.abs(x0 - domX) < Math.abs(x1 - domX) ? x0 : x1;
			double y = Math.abs(y0 - domY) < Math.abs(y1 - domY) ? y0 : y1;
			
			return (1.0-x)*(1.0-x) + 100.0*(y - x*x)*(y - x*x);
		}
		
		
		private double getRandomX()
		{
			return random.nextBoolean() ? x0 : x1;
		}
		
		private double getRandomY()
		{
			return random.nextBoolean() ? y0 : y1;
		}
		

		private void mutate()
		{
			if ( random.nextDouble()<mutationProbability )
			{
				double a = random.nextDouble()*mutationFactor;
				a = random.nextBoolean() ? a : -a;
				x0 = (x0 + a <= 2.0 && x0 + a >= -2.0) ? x0 + a : x0;
			}
			if ( random.nextFloat()<mutationProbability )
			{
				double a = random.nextDouble()*mutationFactor;
				a = random.nextBoolean() ? a : -a;
				x1 = (x1 + a <= 2.0 && x1 + a >= -2.0) ? x1 + a : x1;
			}
			if ( random.nextFloat()<mutationProbability )
			{
				double a = random.nextDouble()*mutationFactor;
				a = random.nextBoolean() ? a : -a;
				y0 = (y0 + a <= 2.0 && y0 + a >= -2.0) ? y0 + a : y0;
			}
			if ( random.nextFloat()<mutationProbability )
			{
				double a = random.nextDouble()*mutationFactor;
				a = random.nextBoolean() ? a : -a;
				y1 = (y1 + a <= 2.0 && y1 + a >= -2.0) ? y1 + a : y1;
			}
		}

		@Override
		public int compareTo(Individual o) 
		{
			if (equals(o))
			{
				return 0;
			}
			if(getFenotyp()<o.getFenotyp())
			{
				return -1;
			}
			if(getFenotyp()>o.getFenotyp())
			{
				return 1;
			}
			if(x0!=o.x0)return x0 < o.x0 ? -1 : 1;
			if(x1!=o.x1)return x1 < o.x1 ? -1 : 1;
			if(y0!=o.y0)return y0 < o.y0 ? -1 : 1;
			if(y1!=o.y1)return y1 < o.y1 ? -1 : 1;
			return 0;
		}

		@Override
		public int hashCode() {
			final int prime = 31;
			int result = 1;
			long temp;
			temp = Double.doubleToLongBits(x0);
			result = prime * result + (int) (temp ^ (temp >>> 32));
			temp = Double.doubleToLongBits(x1);
			result = prime * result + (int) (temp ^ (temp >>> 32));
			temp = Double.doubleToLongBits(y0);
			result = prime * result + (int) (temp ^ (temp >>> 32));
			temp = Double.doubleToLongBits(y1);
			result = prime * result + (int) (temp ^ (temp >>> 32));
			return result;
		}

		@Override
		public boolean equals(Object obj) {
			if (this == obj)
				return true;
			if (obj == null)
				return false;
			if (getClass() != obj.getClass())
				return false;
			Individual other = (Individual) obj;
			if (Double.doubleToLongBits(x0) != Double.doubleToLongBits(other.x0))
				return false;
			if (Double.doubleToLongBits(x1) != Double.doubleToLongBits(other.x1))
				return false;
			if (Double.doubleToLongBits(y0) != Double.doubleToLongBits(other.y0))
				return false;
			if (Double.doubleToLongBits(y1) != Double.doubleToLongBits(other.y1))
				return false;
			return true;
		}

		@Override
		public String toString() {
			double x,y;
			if(codomination)
			{
				x = (x0+x1)/2;
				y = (y0+y1)/2;
			}
			else
			{
				x = Math.abs(x0 - domX) < Math.abs(x1 - domX) ? x0 : x1;
				y = Math.abs(y0 - domY) < Math.abs(y1 - domY) ? y0 : y1;
			}
			
			return "f(  " + x + ",\t" + y + " ) =\t" + getFenotyp();
		}

		@Override
		protected Object clone() throws CloneNotSupportedException {
			return new Individual(this); 
		}
	}
}
