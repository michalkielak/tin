#include "rtp.h"

Rtp::Rtp(unsigned short snr, unsigned int tstamp, unsigned int sc, unsigned short h)
{
	 seq_nr = snr;
	 timestamp = tstamp;
	 ssrc = sc;
	 hard = h;
}


unsigned char * Rtp::serialize_int(unsigned char *buffer, unsigned int value)
{
  /* Write big-endian int value into buffer; assumes 32-bit int and 8-bit char. */
  buffer[0] = value >> 24;
  buffer[1] = value >> 16;
  buffer[2] = value >> 8;
  buffer[3] = value;
  return buffer + 4;
}

unsigned char * Rtp::serialize_short(unsigned char *buffer, unsigned short value)
{
  /* Write big-endian int value into buffer; assumes 32-bit int and 8-bit char. */
  buffer[0] = value >> 8;
  buffer[1] = value;
  return buffer + 2;
}

unsigned char * Rtp::serialize_char(unsigned char *buffer, unsigned char value)
{
  buffer[0] = value;
  return buffer + 1;
}

unsigned char * Rtp::serialize(unsigned char *buffer)
{
  buffer = serialize_short(buffer, this->hard);
  buffer = serialize_short(buffer, this->seq_nr);
  buffer = serialize_int(buffer, this->timestamp);
  buffer = serialize_int(buffer, this->ssrc);
  return buffer;
}