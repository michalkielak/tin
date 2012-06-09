package pl.waw.mizinski.pszt.event;

public class WriteEvent extends PsztEvent
{
	public String message;

	public WriteEvent(String message)
	{
		super();
		this.message = message;
	}
}
