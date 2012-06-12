package pl.waw.mizinski.pszt.model.population;

import java.util.Arrays;
import java.util.Random;
import java.util.concurrent.BlockingQueue;

import pl.waw.mizinski.pszt.event.PsztEvent;
import pl.waw.mizinski.pszt.event.WriteEvent;

public class Population implements Runnable
{
	private final int HOW_MANY_PARTS = Integer.MAX_VALUE;
	private final double part = 4.0 / HOW_MANY_PARTS;
	private final int size;
	private final double mutationProbability;

	private final BlockingQueue<PsztEvent> blockingQueue;
	
	private static Random random = new Random();
	
	public Population(int size, double mutationProbability, BlockingQueue<PsztEvent> blockingQueue)
	{
		super();
		this.size = size;
		this.mutationProbability = mutationProbability;
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
		Individual temp = null;
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
		int mask = getRandomMask();
		int x1 = a.getX() & mask;
		mask = ~mask;
		int x2 = b.getX() & mask;
		mask = getRandomMask();
		int y1 = a.getY() & mask;
		mask = ~mask;
		int y2 = b.getY() & mask;
		int x = x1 | x2;
		int y = y1 | y2;
		Individual individual = new Individual(x,y);
		individual.mutate();
		return individual;
	}
	
	private static int getRandomMask()
	{
		int mask = 0;
		int temp = 1;
		int count = random.nextInt(32);
		for(int i=0; i<count ; ++i)
		{
			mask = mask | temp;
			temp = temp << 1;
		}
		return mask;
		
	}
	
	private static int toGray(int a)
	{
		int b =  a >> 1;
		return a^b;
	}
	
	private int fromGray(int a)
	{
		String from = Integer.toBinaryString(a);
		byte temp = 0;
		int val = 0;
		for (byte b : from.getBytes())
		{
			
			byte i = (byte)(b == '0' ? 0 : 1);
			temp = (byte)( (temp^i)&1 );
			val*=2;
			val+=temp;
		}
		return val;
	}
	
	private double getVariableValue(int x)
	{
		return part*x -2.0;
	}
	
	private static double f(double x, double y)
	{
		return (1.0 - x)*(1.0 - x) + 100.0*(y - x*x)*(y - x*x);
	}
	
	private class Individual implements Comparable<Individual>
	{
		int x , y;
	
		public Individual()
		{
			x = toGray( random.nextInt(HOW_MANY_PARTS) );
			y = toGray( random.nextInt(HOW_MANY_PARTS) );
		}
		
		public Individual(Individual individual)
		{
			super();
			this.x = individual.x;		
			this.y = individual.y;
		}
		
		public Individual(int x, int y)
		{
			super();
			this.x = x;
			this.y = y;
		}

		public double getFenotyp(){
			return( f(getVariableValue( fromGray(x) ), getVariableValue( fromGray(y) )) );
		}

		private void mutate()
		{
			if ( random.nextDouble()<mutationProbability )
			{
				int mask = 1 << random.nextInt(31);
				int temp = mask & x;
				if (temp == 0){
					x = x | mask;
									}
				else
				{
					mask = ~mask;
					x = mask & x;
				}
			}
			if ( random.nextDouble()<mutationProbability )
			{
				int mask = 1 << random.nextInt(31);
				int temp = mask & y;
				if (temp == 0){
					y = mask | y;
				}
				else
				{
					mask = ~mask;
					y = mask & y;
				}
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
			if(x!=o.x)return x < o.x ? -1 : 1;			
			if(y!=o.y)return y < o.y ? -1 : 1;
			return 0;
		}

		@Override
		public String toString() {
			double xx = getVariableValue(fromGray(x));
			double yy = getVariableValue(fromGray(y));
			return "f(  " + xx + ",\t" + yy + " ) =\t" + getFenotyp();
		}

		@Override
		protected Object clone() throws CloneNotSupportedException {
			return new Individual(this); 
		}

		@Override
		public int hashCode()
		{
			final int prime = 31;
			int result = 1;
			result = prime * result + getOuterType().hashCode();
			result = prime * result + x;
			result = prime * result + y;
			return result;
		}

		@Override
		public boolean equals(Object obj)
		{
			if (this == obj)
				return true;
			if (obj == null)
				return false;
			if (getClass() != obj.getClass())
				return false;
			Individual other = (Individual) obj;
			if (!getOuterType().equals(other.getOuterType()))
				return false;
			if (x != other.x)
				return false;
			if (y != other.y)
				return false;
			return true;
		}

		private Population getOuterType()
		{
			return Population.this;
		}

		public int getX()
		{
			return x;
		}

		public int getY()
		{
			return y;
		}
	}
}
