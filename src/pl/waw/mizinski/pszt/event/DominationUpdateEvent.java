package pl.waw.mizinski.pszt.event;

public class DominationUpdateEvent extends PsztEvent
{
	public boolean codomination;
	public double domX;
	public double domY;
	
	public DominationUpdateEvent()
	{
		this.codomination = true;
	}
	
	public DominationUpdateEvent(final double domX, final double domY)
	{
		this.codomination = false;
		this.domX = domX;
		this.domY = domY;
	}
}
