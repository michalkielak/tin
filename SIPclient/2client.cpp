/* UDP client in the internet domain */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fstream>
#include <pthread.h>
#include <signal.h>
#include "rtp.h"
#include <string>
#include <iostream>
using namespace std;
void * ReceiveHandler( void * ld);
void error(const char *);
int rtp_session(int argc, char *argv[]);
/*
int main(int argc, char *argv[])
{
	rtp_session(argc,argv);
 	sleep(8);
	return 0;


}*/
int rtp_session(int argc, const char *argv[])
{
   FILE * file;
   ifstream myReadFile;
   int sock, n;
   int * csock;
   unsigned int length;
   struct sockaddr_in server, from;
   struct hostent *hp;
   Rtp rtp_protocol(1,2,3);// pierwsza wartosc 3 argumentu powinna byc losowa, zapobiega to atakom
   unsigned char buffer[256], * ptr;
   pthread_t thread_id = 0; 
   string str; 
 
   if (argc != 3) { printf("Usage: server port\n");
                    exit(1);
   }
   sock= socket(AF_INET, SOCK_DGRAM, 0);
   if (sock < 0) error("socket");

   server.sin_family = AF_INET;
   hp = gethostbyname(argv[1]);
   if (hp==0) error("Unknown host");

   bcopy((char *)hp->h_addr, 
        (char *)&server.sin_addr,
         hp->h_length);
   server.sin_port = htons(atoi(argv[2]));
   length=sizeof(struct sockaddr_in);

   // utworzenie watku sluchajacego
   csock = (int*)malloc(sizeof(int));
   *csock = sock;
   pthread_create(&thread_id,0,&ReceiveHandler, (void*)csock );
//   pthread_detach(thread_id);

   int i =1; // wartosc seq_nr, jeszcze trzeba reszte dodac (?)
   myReadFile.open("text.txt", ios::in | ios::binary);
   if(myReadFile.is_open()){
       while(!myReadFile.eof()) {
            bzero(buffer,sizeof(buffer));
           rtp_protocol.seq_nr = i; 
	   // serializacja naglowka do bufora 
	   ptr = rtp_protocol.serialize(buffer); 
           myReadFile.read((char *) ptr, sizeof(buffer) - sizeof(rtp_protocol));
           getline(myReadFile,str);  // wczytanie wiersza, narazie musi byc  mniejszy niz 256 - sizeof(rtp_protocol), bo przepelni sie bufor. Poza tym nie obluguje binarnych
           ptr += myReadFile.gcount(); // przesuniecie wskaznik bufora o tyle ile znakow wczytano
           // wyslanie
	   n=sendto(sock,buffer,
		    ptr-buffer,0,(const struct sockaddr *)&server,length);
	   if (n < 0) error("Sendto");
//	   printf("Wyslalem z watku glownego\n");
           ++i; // zwiekszenie seq_nr
       }
	cout << "Glowny:" << i << endl;
   }
   if(pthread_cancel(thread_id))
	error("Failed to kill the threat\n");
   if(pthread_join(thread_id, NULL))
	error("Failed to wait for termination of the threat\n");

  // myReadFile.close();
   close(sock);
   return 0;
}

void * ReceiveHandler( void * ld)
{
    int *csock = (int*)ld;
    unsigned int length;
    char buffer[1024];
    int buffer_len = 1024;
    int bytecount;
    struct sockaddr_in from;
   length=sizeof(struct sockaddr_in);
    int i =0;
    while(true){
        bytecount = recvfrom(*csock,buffer,1024,0,(struct sockaddr *)&from, &length);
        if (bytecount < 0) error("recvfrom");
	cout << "Dostalem odpowiedz:"<< i << endl;
  	//write(1,"Got an ack: ",12);

	i++;	//write(1,buffer,bytecount);
    } 
}
void error(const char *msg)
{
    perror(msg);
    exit(0);
}



