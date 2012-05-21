#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <qsettings.h>
#include <arpa/inet.h>

#include "extmessagesocket.h"

int EXTMessageSocket::port_count=0;

EXTMessageSocket::EXTMessageSocket( bool usetcp, int min,int max  )
{
//	int	reuse = 1;
	if(usetcp) {
	type = SocketTCP;
		if ( ( socketfd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 ) {
			perror( "EXTMessageSocket::EXTMessageSocket(): socket() failed" );
		}
		didcomplain = false;
	} else {
		type = SocketUDP;
		if ( ( socketfd = socket( AF_INET, SOCK_DGRAM, 0 ) ) == -1 ) {
			perror( "EXTMessageSocket::EXTMessageSocket(): socket() failed" );
		}
	didcomplain = false;
	}
	minP=min;
	maxP=max;

}


EXTMessageSocket::~EXTMessageSocket( void )
{
	close( socketfd );
}


int EXTMessageSocket::listenOnEvenPort(void)
{
	int bind_err;
	
	if (minP % 2) minP++;
	/* Out of range ? Signal error */
	if (minP > maxP) goto error;
	//minP+=port_count;
	port_count+=2;
 	/* Out of range ? Signal error */
	if (minP > maxP) goto error;

loop:
	socketaddress.sin_family = AF_INET;
	socketaddress.sin_port = htons(minP);
	socketaddress.sin_addr.s_addr = INADDR_ANY;
	bzero(&(socketaddress.sin_zero), 8);
	bind_err = bind ( socketfd, (struct sockaddr*)&socketaddress, sizeof(struct sockaddr));
	if (bind_err == -1) {
		minP += 2;
		if (minP <= maxP) goto loop;
		else goto error;
	}
	bound = true;
	ourport = minP;
	minP+=2;
	if(type== SocketTCP)
	{
		if ( ::listen( socketfd, 10 ) == -1 ) return -1;

	}
	return 0;
error:
	perror("EXTMessageSocket::listen(): Can't find a free port in specified range");
	return -1;

}
