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

using namespace std;

void * ReceiveHandler( void * ld);
void error(const char *);

int main(int argc, char *argv[])
{
   FILE * file;
   ifstream myReadFile;
   int sock, n;
   int * csock;
   unsigned int length;
   struct sockaddr_in server, from;
   struct hostent *hp;
   char buffer[256];
   pthread_t thread_id = 0;  
 
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
   pthread_detach(thread_id);



   // pobranie do bufora
 //  printf("Please enter the message: ");
   bzero(buffer,256);
  // fgets(buffer,255,stdin);
   myReadFile.open("text.txt");
   if(myReadFile.is_open()){
       while(!myReadFile.eof()) {
           myReadFile >> buffer;
           // wyslanie
	   n=sendto(sock,buffer,
		    strlen(buffer),0,(const struct sockaddr *)&server,length);
	   if (n < 0) error("Sendto");
       }
   }
   if(pthreat_kill(thread_id,1))
	error("Failed to kill the threat\n");
   myReadFile.close();
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
    while(true){
        bytecount = recvfrom(*csock,buffer,1024,0,(struct sockaddr *)&from, &length);
        if (bytecount < 0) error("recvfrom");
	write(1,"Got an ack: ",12);
	write(1,buffer,bytecount);
    } 
}
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

/*void create_rtp(unsigned char * buffer, int size)
{
    unsigned char buf = 0;

    bzero(buffer, size);
    buffer[0]


}*/
