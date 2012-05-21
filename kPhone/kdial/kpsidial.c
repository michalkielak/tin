#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>


main(int argc, 	char *argv[]) {
	char *mess[1000];
	int sock,serlen,mlen;
	struct sockaddr_un client, server;
        struct sockaddr *part;
	if (argc <= 1) {
	    printf(" at least 1 parameter\n");
	    _exit(1);
	}
	
	/* Create socket on which to send. */
	sock = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("opening datagram socket");
		_exit(1);
	}
	
	/* Construct name of socket to send to. */
	client.sun_family = AF_UNIX;
	strcpy(client.sun_path, "/tmp/kpsidg.XXXXXX");
	mkstemp(client.sun_path);
	unlink(client.sun_path);
        int l = strlen(client.sun_path)+sizeof(client.sun_family); 
        if(bind (sock,(struct sockaddr *)&client, l) < 0) {
            perror("bind");
	    close (sock);
	    unlink(client.sun_path);
	    _exit(1);
	}
	
	/*prep endpoint*/
	server.sun_family = AF_UNIX;
	strcpy(server.sun_path, "/tmp/kpsidg");
        serlen = strlen(server.sun_path)+sizeof(server.sun_family); 
	
	/* Send message. */
	
	if(argc == 2) {
	    sprintf((char *)&mess,"A%s",argv[1]);
	    mlen = strlen(argv[1]) + 1;
	} else  {
		sprintf((char *)&mess,"%s%s",argv[1],argv[2]);
		mlen = strlen(argv[1]) + strlen(argv[2]);
	}
		printf ("-> %d %s\n",mlen,&mess[0]);
	if (sendto(sock, &mess[0], mlen,  0, (struct sockaddr *)&server, serlen) < 0) 
         { 
        perror("sending datagram message"); 
        }
	close(sock);
	unlink(client.sun_path);
}
