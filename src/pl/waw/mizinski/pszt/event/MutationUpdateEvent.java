package pl.waw.mizinski.pszt.event;

public class MutationUpdateEvent extends PsztEvent
{
	public double probability;
	
	public MutationUpdateEvent(double probability)
	{
		super();
		this.probability = probability;
	}
}
