#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "parser.cpp"
#include "md5.h"
#include "messages.cpp"
#include "2client.cpp"
//#define SRV_IP otherIp.c_str()
#define SRV_IP "194.29.169.4"
#define BUFLEN 10240
#define NPACK 10
#define PORT 8060
using namespace std;


void diep(const char* s)
{
  perror(s);
  exit(1);
}

int main(int argc, char* argv[]) {
	
	int i=0;
	string myLogin, myIp, otherLogin, otherIp, serverIp;
	int code;
	char *toTag, *via, *from, *to, *c_seq, *call_id;
	char *parsedIp;
	
	if (argc!=2)
	{
		diep("nie poprawna nazwa pliku");
	}
	//osobny zakres do obslugi pliku
	{
		ifstream in(argv[1]);
		if(!in)
		{
			diep("nie poprawna nazwa pliku");
		}
		in >> myLogin;
		in >> myIp;
		in >> otherLogin;
		in >> otherIp;
		serverIp = "194.29.169.4";
	}
	
	cout << myLogin << "@" << myIp <<" " << otherLogin << "@" << otherIp <<endl;
	cout << serverIp << endl;

	Messages *message = new Messages(myLogin, myIp, otherLogin, /*otherIp,*/ serverIp);
	//message->init();
	struct sockaddr_in si_other;
	int s, slen=sizeof(si_other);
	char buf[BUFLEN];

	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		diep("socket");

	string registerMsg = message->getRegisterMsg();

	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	si_other.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(s, (struct sockaddr*)&si_other, sizeof(si_other))==-1)
		diep("bind");

	if (inet_aton(SRV_IP, &si_other.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}


/**********************stad robimy register******/
	int send_size=0;
	printf("Sending REGISTER message\n");
	send_size=sendto(s, registerMsg.c_str(), registerMsg.length(), 0, (struct sockaddr*)&si_other, slen);
	cout<<"Send "<<send_size<<" bytes"<<endl;
	if (send_size==-1)
	diep("sendto()");

	do
	{
		if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, (unsigned int*)&slen)==-1)
		{
			diep("recvfrom()");
		}
		printf("Received packet from %s:%d\nData: %s\n\n",
		inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
		parseMsg(buf, strlen(buf), &code, &parsedIp, &toTag);
	}
	while(code!=401);
	
	char *r;
	string nonce = getNonce(buf,strlen(buf),&r);
	string realm = r;
	
	nonce.erase(nonce.begin());
	nonce.erase(--nonce.end());
	realm.erase(realm.begin());
	realm.erase(--realm.end());
	
	cout << nonce << " " <<realm << endl;
// realm="tele.pw.edu.pl"; nonce="a6d9f03e06c0d14955071d3f0bb1ee923e728657";
 
	string s1 = string(myLogin + ":" + realm + ":" + myLogin);
	string s2 = string(string("REGISTER:sip:") + serverIp);
	char b1[16];
	char b2[16];
	char ha1[33];
	char ha2[33];
	md5_buffer(s1.c_str(),s1.length(),(void*)b1);
	md5_buffer(s2.c_str(),s2.length(),(void*)b2);
	md5_sig_to_string((void*)b1, ha1, 33);
	md5_sig_to_string((void*)b2, ha2, 33);
	
	char b3[16];
	string s3 = string(string(ha1) + ":" + nonce + ":" + string(ha2));
	md5_buffer(s3.c_str(),s3.length(),(void*)b3);
	
	char response[33];
	md5_sig_to_string((void*)b3, response, 33);



	 printf("Sending REGISTER message with authentication\n");
	 
	 registerMsg = message->getRegisterAuthMsg(nonce, response);
	 cout<<registerMsg<<endl;
	send_size=sendto(s, registerMsg.c_str(), registerMsg.length(), 0, (struct sockaddr*)&si_other, slen);
	cout<<"Send "<<send_size<<" bytes"<<endl;
	if (send_size==-1)
	{
		diep("sendto()");
	}


		
	if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, (unsigned int*)&slen)==-1)
	{
		diep("recvfrom()");
	}
	printf("Received packet from %s:%d\nData: %s\n\n",
	inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
		

	
	 string inviteMsg = message->getInviteMsg();
	 send_size=sendto(s, inviteMsg.c_str(), inviteMsg.length(), 0, (struct sockaddr*)&si_other, slen);
		   cout<<"Send "<<send_size<<" bytes"<<endl;
		   if (send_size==-1)
		   {
			 diep("sendto()");
		   }
		   while(1){
             if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, (unsigned int*)&slen)==-1)
			 {
				diep("recvfrom()");
			 }

		   	parseMsg(buf, strlen(buf), &code, &parsedIp, &toTag);
		   	cout<<"Received code "<<code<<endl;
			if(code==200)
		   		break;
		   }


 	string ackMsg = message->getAckMsg(string(toTag));
	send_size=sendto(s, ackMsg.c_str(), ackMsg.length(), 0, (struct sockaddr*)&si_other, slen);
	cout<<"Send "<<send_size<<" bytes"<<endl;
	if (send_size==-1)
		diep("sendto()");



	if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, (unsigned int*)&slen)==-1)
	{
		diep("recvfrom()");
	}
	parseMsg(buf, strlen(buf), &code, &parsedIp, &toTag);
	cout<<"Received code "<<code<<endl;

	sleep(2);

	if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, (unsigned int*)&slen)==-1)
	{
		diep("recvfrom()");
							
	}
	parseMsg(buf, strlen(buf), &code, &parsedIp, &toTag);
	cout<<"Received code "<<code<<endl;
	if (code == 0) 
	{
		getOptions(buf, strlen(buf), &via, &from, &to, &call_id, &c_seq);
		string okMsg = message->getOkMsg(string(via), string(from), string(to), string(call_id), string(c_seq));
		send_size=sendto(s, okMsg.c_str(), okMsg.length(), 0, (struct sockaddr*)&si_other, slen);
		cout<<"Send "<<send_size<<" bytes"<<endl;
		if (send_size==-1)
		{
			diep("sendto()");
		}
		
	}
	

	sleep(10);
 	
	string byeMsg = message->getByeMsg(string(toTag));
	send_size=sendto(s, byeMsg.c_str(), byeMsg.length(), 0, (struct sockaddr*)&si_other, slen);
	cout<<"Send "<<send_size<<" bytes"<<endl;
	if (send_size==-1)
	{
		diep("sendto()");
	}

	byeMsg = message->getByeMsg(string(to));
	send_size=sendto(s, byeMsg.c_str(), byeMsg.length(), 0, (struct sockaddr*)&si_other, slen);
	cout<<"Send "<<send_size<<" bytes"<<endl;
	if (send_size==-1)
		diep("sendto()");


	while(1)
	{
		 if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, (unsigned int*)&slen)==-1)
						diep("recvfrom()");
		parseMsg(buf, strlen(buf), &code, &parsedIp, &toTag);
		cout<<"Received code "<<code<<endl;
		if(code==200)
		{
			break;
		}
	}



	 close(s);


	return 0;
}

void *server(void *threadid)
{
	struct sockaddr_in si_me, si_other;
	        int s, i, slen=sizeof(si_other);
	        char buf[BUFLEN];

	        if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	          diep("socket");

	        memset((char *) &si_me, 0, sizeof(si_me));
	        si_me.sin_family = AF_INET;
	        si_me.sin_port = htons(PORT);
	        si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	        if (bind(s, (struct sockaddr*)&si_me, sizeof(si_me))==-1)
	            diep("bind");

	        for (i=0; i<NPACK; i++) {
	          if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, (unsigned int*)&slen)==-1)
	            diep("recvfrom()");
	          printf("Received packet from %s:%d\nData: %s\n\n",
	                 inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
	        }

	        close(s);
	pthread_exit(NULL);
}
