package pl.waw.mizinski.pszt;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

import pl.waw.mizinski.pszt.controller.Controller;
import pl.waw.mizinski.pszt.event.*;
import pl.waw.mizinski.pszt.model.Model;
import pl.waw.mizinski.pszt.view.View;

public class Pszt
{
	public static void main(String[] args)
	{
		try
		{
			BlockingQueue<PsztEvent> blockingQueue = new LinkedBlockingQueue<PsztEvent>();
			View view = new View(blockingQueue);
			Model model = new Model(blockingQueue);
			Controller controller = new Controller(view, model, blockingQueue);
			controller.work();
		}
		catch (Exception e)
		{
			e.printStackTrace();
			System.exit(1);
		}
	}
}
