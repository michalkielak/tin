#ifndef RTPCLIENT_H
#define RTPCLIENT_H
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

void * ReceiveHandler( void * ld);
void error(const char *);
int rtp_session(int argc, const char *argv[], const char * fileName);

void diep(const char* s);

#endif
