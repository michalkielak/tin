package pl.waw.mizinski.pszt.event;

public class MutationUpdateEvent extends PsztEvent
{
	public double probability;
	public double factor;
	
	public MutationUpdateEvent(double probability, double factor)
	{
		super();
		this.probability = probability;
		this.factor = factor;
	}
}
