#ifndef RTPDATAHEADER_H
#define RTPDATAHEADER_H

#include "../config.h"

// For 32bit intel machines
typedef short int16;
typedef int   int32;
typedef unsigned int u_int32;
typedef unsigned short u_int16;


//#define RTP_BIG_ENDIAN 0
//#define RTP_LITTLE_ENDIAN 1

//  RTP data header
typedef __attribute__ ((__packed__)) struct {
#ifdef ENDIAN
	unsigned int version:2;         // protocol version
	unsigned int p:1;               // padding flag
	unsigned int x:1;               // header extension flag
	unsigned int cc:4;              // CSRC count
	unsigned int m:1;               // marker bit
	unsigned int pt:7;              // payload type
#else 
	unsigned int cc:4;              // CSRC count
	unsigned int x:1;               // header extension flag
	unsigned int p:1;               // padding flag
	unsigned int version:2;         // protocol version
	unsigned int pt:7;              // payload type
	unsigned int m:1;               // marker bit
#endif
	unsigned int seq:16;            // sequence number
	u_int32 ts;                     // timestamp 32bits
	u_int32 ssrc;                   // synchronization source
} rtp_hdr_t;

typedef struct {
//#elif RTP_LITTLE_ENDIAN .. only coded for this
	unsigned int id:8;				// dtmf code
	unsigned int vol:6;				// volume
	unsigned int res:1;				// reserver
	unsigned int eoe:1;				// end of event
	unsigned int dur:16;			// duration
} rtp_dtmf_t;

#endif
