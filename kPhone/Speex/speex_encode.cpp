
#include <stdlib.h>
#include <stdio.h>
#include "speex_define.h"
#include "speex.h"


void initspeexEncode(                   /* (o) Number of bytes encoded */ 
    speex_Enc_Inst_t *global_vars   /* (i/o) Encoder instance */ 
)
{
	global_vars->state = speex_encoder_init(&speex_nb_mode);
	/*Set the quality to 8 (15 kbps)*/
	int tmp=8;
	speex_encoder_ctl(global_vars->state, SPEEX_SET_QUALITY, &tmp);
	/*Initialization of the structure that holds the bits*/
	speex_bits_init(&global_vars->bits);

}
void termspeexEncode(                   /* (o) Number of bytes encoded */ 
    speex_Enc_Inst_t *global_vars   /* (i/o) Encoder instance */ 
)
{
	speex_bits_destroy(&global_vars->bits);
	speex_encoder_destroy(global_vars->state);
}

int speex_encode (
	speex_Enc_Inst_t *global_vars,
    short *decoded_data,			/* input */
    short *encoded_data 			/* output */
)
{

//	short in[FRAME_SIZE];
	float input[FRAME_SIZE];
	char cbits[200];
	int nbBytes;
	short * ifc;
	int i;
	
	ifc = decoded_data;

	for (i=0;i<FRAME_SIZE;i++) {
		input[i] = *ifc;
		ifc++;
	}
	
	/*Flush all the bits in the struct so we can encode a new frame*/
	speex_bits_reset(&global_vars->bits);
	/*Encode the frame*/
	speex_encode(global_vars->state, input, &global_vars->bits);
	/*Copy the bits to an array of char that can be written*/
	nbBytes = speex_bits_write(&global_vars->bits, cbits, 200);
	/*Write the compressed data*/

	char * ifc2;

	ifc2 = (char*) encoded_data;
	for (i=0; i<nbBytes;i++) {
		*ifc2 = cbits [i];
		ifc2++;
		}
	//fwrite(cbits, 1, nbBytes, stdout);
	return nbBytes;
}
