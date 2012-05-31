#ifndef RTP_H
#define RTP_H
#include <iostream>


class Rtp
{
public:    unsigned short hard;
    unsigned short seq_nr;
    unsigned int timestamp;
    unsigned int ssrc;

    

    Rtp(unsigned short snr, unsigned int tstamp, unsigned int sc, unsigned short h = 0x8070);
private:    
	unsigned char * serialize_int(unsigned char *buffer, unsigned int value);
    unsigned char * serialize_char(unsigned char *buffer, unsigned char value);
    unsigned char * serialize_short(unsigned char *buffer, unsigned short value);
public:    
	unsigned char * serialize(unsigned char *buffer);
 
};

#endif
