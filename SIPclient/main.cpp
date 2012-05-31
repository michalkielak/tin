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
#include "parser.h"
#include "md5.h"
#include "messages.h"
#include "rtpclient.h"
#define SRV_IP serverIp.c_str()
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
	string myLogin, password, myIp, otherLogin, otherIp, serverIp;
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
		in >> password;
		in >> otherLogin;
		in >> serverIp;
	}
	
	//cout << polaczenie 
	
	Messages *message = new Messages(myLogin, otherLogin, serverIp);
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

	cout << "nawiazano polaczenie z serwerem" << endl;
	cout << "wysylanie wiadomosci REGISTER" << endl;

	int send_size=0;
	send_size=sendto(s, registerMsg.c_str(), registerMsg.length(), 0, (struct sockaddr*)&si_other, slen);
	cout<<"Wyslano "<<send_size<<" bajtow"<<endl;
	if (send_size==-1)
	diep("sendto()");

	do
	{
		if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, (unsigned int*)&slen)==-1)
		{
			diep("recvfrom()");
		}
		parseMsg(buf, strlen(buf), &code, &parsedIp, &toTag);
		
		cout << "otrzymano wiadomosc o kodzie: " << code << endl;
	}
	while(code!=401);
	
	char *r;
	string nonce = getNonce(buf,strlen(buf),&r);
	string realm = r;
	
	nonce.erase(nonce.begin());
	nonce.erase(--nonce.end());
	realm.erase(realm.begin());
	realm.erase(--realm.end());
	string s1 = string(myLogin + ":" + realm + ":" + password);
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
	
	 cout << "wysylanie wiadomosci REGISTER z autoryzacja" << endl;
	 
	 registerMsg = message->getRegisterAuthMsg(nonce, response);
	send_size=sendto(s, registerMsg.c_str(), registerMsg.length(), 0, (struct sockaddr*)&si_other, slen);
	if (send_size==-1)
	{
		diep("sendto()");
	}
	cout<<"Wyslano "<<send_size<<" bajtow"<<endl;

		
	if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, (unsigned int*)&slen)==-1)
	{
		diep("recvfrom()");
	}

	parseMsg(buf, strlen(buf), &code, &parsedIp, &toTag);
	cout << "otrzymano wiadomosc o kodzie: " << code << endl;
		
	cout << "Wysylanie wiadomosci INVITE" << endl;
	 string inviteMsg = message->getInviteMsg();
	 send_size=sendto(s, inviteMsg.c_str(), inviteMsg.length(), 0, (struct sockaddr*)&si_other, slen);
	cout<<"Wyslano "<<send_size<<" bajtow"<<endl;
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
	cout << "otrzymano wiadomosc o kodzie: " << code << endl;
	if(code==200)
		break;
	}
	
	cout << "Wysylanie wiadomosci ACK" << endl;
 	string ackMsg = message->getAckMsg(string(toTag));
	send_size=sendto(s, ackMsg.c_str(), ackMsg.length(), 0, (struct sockaddr*)&si_other, slen);
	cout<<"Wyslano "<<send_size<<" bajtow"<<endl;
	if (send_size==-1)
	{
		diep("sendto()");
	}
	if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, (unsigned int*)&slen)==-1)
	{
		diep("recvfrom()");
	}
	parseMsg(buf, strlen(buf), &code, &parsedIp, &toTag);
	cout << "otrzymano wiadomosc o kodzie: " << code << endl;

	getIp(buf, strlen(buf), &to);
	otherIp = to;
	cout <<"uztkownik zdalny( " << otherLogin << " ) ma IP: "<< otherIp << endl;

	parseMsg(buf, strlen(buf), &code, &parsedIp, &toTag);
	cout << "otrzymano wiadomosc o kodzie: " << code << endl;
	if (code == 0) 
	{
		cout<<"Wysylanie wiadomosci OK" <<endl;
		getOptions(buf, strlen(buf), &via, &from, &to, &call_id, &c_seq);
		string okMsg = message->getOkMsg(string(via), string(from), string(to), string(call_id), string(c_seq));
		send_size=sendto(s, okMsg.c_str(), okMsg.length(), 0, (struct sockaddr*)&si_other, slen);
		cout<<"Wyslano "<<send_size<<" bajtow"<<endl;
		if (send_size==-1)
		{
			diep("sendto()");
		}
	}
	
 	const char * tab[3];
 	tab[0] = argv[0];
 	tab[1] = otherIp.c_str();
	tab[2] = string("7078").c_str();
	cout<< "Rozpoczynam sesje RTP z uzytkownikiem: "+otherLogin + "@" + serverIp + " na hoscie: " + otherIp <<endl;  
	rtp_session(3,tab);
	cout << "Zakonczenie sesji RTP" <<endl;
	
	cout <<"Wysylanie wiadomosci BYE" << endl;
	string byeMsg = message->getByeMsg(string(toTag));
	send_size=sendto(s, byeMsg.c_str(), byeMsg.length(), 0, (struct sockaddr*)&si_other, slen);
	cout<<"Wyslano "<<send_size<<" bajtow"<<endl;
	if (send_size==-1)
	{
		diep("sendto()");
	}

	cout <<"Wysylanie wiadomosci BYE" << endl;
	byeMsg = message->getByeMsg(string(to));
	send_size=sendto(s, byeMsg.c_str(), byeMsg.length(), 0, (struct sockaddr*)&si_other, slen);
	cout<<"Wyslano "<<send_size<<" bajtow"<<endl;
	if (send_size==-1)
		diep("sendto()");

	while(1)
	{
		 if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, (unsigned int*)&slen)==-1)
						diep("recvfrom()");
		parseMsg(buf, strlen(buf), &code, &parsedIp, &toTag);
		cout << "otrzymano wiadomosc o kodzie: " << code << endl;
		if(code==200)
		{
			break;
		}
	}
	close(s);
	cout<< "Zamknieto polaczenie z serwerem" << endl;
	return 0;
}