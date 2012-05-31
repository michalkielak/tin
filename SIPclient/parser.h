#ifndef PARSER_H
#define PARSER_H
#include <stdlib.h>
#include <sys/time.h>
#include <osip2/osip.h>

/* argumenty:
	msg - wiadomosc
	len - dlugosc wiadomosci
	code - zwraca kod wiadomosci
	contact_ip - zwraca adres ip z naglowka contact
*/   
int parseMsg(char *msg, size_t len, int *code, char **contact_ip, char **tag);

int getOptions(char *msg, size_t len, char **via, char** from, char ** to, char **call_id, char **c_seq);

int getIp(char *msg, size_t len, char **ip);

char* getNonce(char* msg, size_t len, char **realm);


#endif