#ifndef SPEEX_DEFINE_H_INCLUDED
#define SPEEX_DEFINE_H_INCLUDED
#include "speex.h"

#define FRAME_SIZE 160
/* at least 40 bits in encoded speex frame */
#define MIN_SPEEX_FRAME_SIZE  40 

typedef struct speex_Enc_Inst_t_ { 
/*Holds the state of the encoder*/
void *state;
/*Holds bits so they can be read and written to by the Speex routines*/
SpeexBits bits;

} speex_Enc_Inst_t;


typedef struct speex_Dec_Inst_t_ { 
/*Holds the state of the decoder*/
void *state;
/*Holds bits so they can be read and written to by the Speex routines*/
SpeexBits bits;

} speex_Dec_Inst_t; 

int speex_encode (
	speex_Enc_Inst_t *b,
    short *,			/* output */
    short *				/* input */
);
int speex_decode (
	speex_Dec_Inst_t *,
    short *,			/* input */
    short *,				/* output */
	int
);
void initspeexEncode(                   /* (o) Number of bytes encoded */ 
    speex_Enc_Inst_t *speexenc_inst   /* (i/o) Encoder instance */ 
); 
void initspeexDecode(                   /* (o) Number of bytes encoded */ 
    speex_Dec_Inst_t *speexdec_inst   /* (i/o) Encoder instance */ 
); 
void termspeexEncode(                   /* (o) Number of bytes encoded */ 
    speex_Enc_Inst_t *speexenc_inst   /* (i/o) Encoder instance */ 
); 
void termspeexDecode(                   /* (o) Number of bytes encoded */ 
    speex_Dec_Inst_t *speexdec_inst   /* (i/o) Encoder instance */ 
); 
#endif //SPEEX_DEFINE_H_INCLUDED

