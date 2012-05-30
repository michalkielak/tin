#ifndef client
#define client     
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
#include <time.h>
using namespace std;
void * ReceiveHandler( void * ld);
void error(const char *);
int rtp_session(int argc, const char *argv[]);
/*
int main(int argc, char *argv[])
{
	rtp_session(argc,argv);
 	sleep(8);
	return 0;


}*/
void diep(const char* s);

int rtp_session(int argc, const char *argv[])
{
   FILE * file;
   ifstream myReadFile;
   int sock, n;
   int * csock;
   clock_t begin;
   clock_t current; 
   unsigned int length;
   struct sockaddr_in server, from;
   struct hostent *hp;
   Rtp rtp_protocol(1,2,3);// pierwsza wartosc 3 argumentu powinna byc losowa, zapobiega to atakom
   unsigned char buffer[256], * ptr;
   pthread_t thread_id = 0; 
   string str; 
  struct sockaddr_in si_other;
        int s, slen=sizeof(si_other);
        if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
               cout << "socketuj" << endl;

        memset((char *) &si_other, 0, sizeof(si_other));
        si_other.sin_family = AF_INET;
        si_other.sin_port = htons(atoi(argv[2]));
        cout << argv[2] << endl;
        si_other.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(s, (struct sockaddr*)&si_other, sizeof(si_other))==-1)
                diep (" bind");

        if (inet_aton(argv[1], &si_other.sin_addr)==0) {
                fprintf(stderr, "inet_aton() failed\n");
                exit(1);
        }

   // utworzenie watku sluchajacego
   csock = (int*)malloc(sizeof(int));
   *csock = s;
   pthread_create(&thread_id,0,&ReceiveHandler, (void*)csock );

   int i =1; // wartosc seq_nr, jeszcze trzeba reszte dodac (?)
   begin = clock();
   myReadFile.open("text.txt", ios::in | ios::binary);
   if(myReadFile.is_open()){
       while(!myReadFile.eof()) {
            
//            cout <<(long) ( clock() - begin)<< endl;
            bzero(buffer,sizeof(buffer));
           rtp_protocol.seq_nr = i; 
	   // serializacja naglowka do bufora 
	   ptr = rtp_protocol.serialize(buffer); 
           
           myReadFile.read((char *) ptr, sizeof(buffer) - sizeof(rtp_protocol));
           getline(myReadFile,str);  // wczytanie wiersza, narazie musi byc  mniejszy niz 256 - sizeof(rtp_protocol), bo przepelni sie bufor. Poza tym nie obluguje binarnych
           ptr += myReadFile.gcount(); // przesuniecie wskaznik bufora o tyle ile znakow wczytano
           // wyslanie
           
	   n=sendto(s,buffer,
		    ptr-buffer,0,(struct sockaddr *)&si_other,sizeof(si_other));
	   if (n < 0) error("Sendto");
           ++i; // zwiekszenie seq_nr
	struct timespec sleepTime;
	struct timespec returnTime;
	sleepTime.tv_sec = 0;
	sleepTime.tv_nsec = 300000;
	nanosleep(&sleepTime, &returnTime);
//	sleep(2);		   

//usleep(125);
       }
   }
   if(pthread_cancel(thread_id))
	error("Failed to kill the threat\n");
   if(pthread_join(thread_id, NULL))
	error("Failed to wait for termination of the threat\n");

  // myReadFile.close();
   close(s);
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
 //       cout << "Odebralem" << endl;
        bytecount = recvfrom(*csock,buffer,1024,0,(struct sockaddr *)&from, &length);
        if (bytecount < 0) error("recvfrom");
//	cout << "Dostalem odpowiedz:"<< i << endl;
  	//write(1,"Got an ack: ",12);

	i++;	//write(1,buffer,bytecount);
    } 
}
void error(const char *msg)
{
    perror(msg);
    exit(0);
}


#endif
