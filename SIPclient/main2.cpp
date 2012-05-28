#include <iostream>
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
#include "udpmessagesocket.h"
#include "messages.cpp"
#define SRV_IP "192.168.47.146"
#define BUFLEN 512
#define NPACK 10
#define PORT 5060
using namespace std;

void *server(void *threadid);

void diep(char *s)
{
  perror(s);
  exit(1);
}

int main() {

	int i=0;
	Messages *message = new Messages();
	message->init();
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

	 int send_size=0;
	   /*printf("Sending REGISTER message\n");
	   send_size=sendto(s, registerMsg.c_str(), registerMsg.length(), 0, (struct sockaddr*)&si_other, slen);
	   cout<<"Send "<<send_size<<" bytes"<<endl;
	   if (send_size==-1)
		 diep("sendto()");

	 for(int j=0; j<2; j++){
	 if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, (unsigned int*)&slen)==-1)
					diep("recvfrom()");
	printf("Received packet from %s:%d\nData: %s\n\n",
	inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);}





	 printf("Sending REGISTER message with authentication\n Enter nonce code\n");
	 string nonce;
	 cin>>nonce;
	 registerMsg = message->getRegisterAuthMsg(nonce);
	 cout<<registerMsg<<endl;
		   send_size=sendto(s, registerMsg.c_str(), registerMsg.length(), 0, (struct sockaddr*)&si_other, slen);
		   cout<<"Send "<<send_size<<" bytes"<<endl;
		   if (send_size==-1)
			 diep("sendto()");

		 for(int j=0; j<2; j++){
		 if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, (unsigned int*)&slen)==-1)
						diep("recvfrom()");
		printf("Received packet from %s:%d\nData: %s\n\n",
		inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);}

	 //sleep(5);*/
	 string inviteMsg = message->getInviteMsg();
	 send_size=sendto(s, inviteMsg.c_str(), inviteMsg.length(), 0, (struct sockaddr*)&si_other, slen);
		   cout<<"Send "<<send_size<<" bytes"<<endl;
		   if (send_size==-1)
			 diep("sendto()");

		   int *code;
		   char **parsedIp;
		   while(1){
		   	 if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, (unsigned int*)&slen)==-1)
		   					diep("recvfrom()");
		   	printf("Received packet from %s:%d\nData: %s\n\n",
		   	inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
		   	parseMsg(&(buf.c_str()), buf.length(), code, parsedIP);
		   	if(*code==200)
		   		break;
		   	else
		   		cout<<"Received code "<<*code<<endl;
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
