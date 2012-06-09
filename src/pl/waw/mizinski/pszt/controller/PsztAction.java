package pl.waw.mizinski.pszt.controller;

import pl.waw.mizinski.pszt.event.PsztEvent;


public interface PsztAction
{
	abstract public void go(PsztEvent e);
}
