package pl.waw.mizinski.pszt.event;

public class SizeUpdateEvent extends PsztEvent
{
	public int size;

	public SizeUpdateEvent(int size)
	{
		super();
		this.size = size;
	}
}
