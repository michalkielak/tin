#ifndef MESSAGES_H
#define MESSAGES_H

#include <iostream>
#include <cstring>
#include <sstream>
#include <stdio.h>      
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <cstring> 
#include <arpa/inet.h>
#include <string>

class Messages{
	std::string registerMsg;
	std::string registerAuthMsg;
	std::string msg3, msg2;
	std::string invite, invite2, invite3;
	std::string ack, bye;
	std::string ok;
	std::string myIp, serverIp, myLogin, otherLogin;
	std::string contentSize;
	std::string getMyIp ();
	
public:
	Messages(std::string myLogin, std::string otherLogin, std::string serverIp);
	


	std::string getRegisterMsg();
	std::string getRegisterAuthMsg(std::string nonce, std::string response);
	std::string getInviteMsg();


	std::string getAckMsg(std::string toTag);
	std::string getByeMsg(std::string toTag);

	std::string getOkMsg(std::string via, std::string from, std::string to, std::string call_id, std::string c_seq);


};

#endif