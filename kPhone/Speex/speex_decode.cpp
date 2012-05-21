
#include <stdlib.h>
#include <stdio.h>

#include "speex_define.h"

void initspeexDecode(                   /* (o) Number of bytes encoded */ 
    speex_Dec_Inst_t *global_vars   /* (i/o) Encoder instance */ 
)
{
	int tmp;
/*Create a new decoder state in narrowband mode*/
	global_vars->state = speex_decoder_init(&speex_nb_mode);
/*Set the perceptual enhancement on*/
	tmp=1;
	speex_decoder_ctl(global_vars->state, SPEEX_SET_ENH, &tmp);
	/*Initialization of the structure that holds the bits*/
	speex_bits_init(&global_vars->bits);

}
void termspeexDecode(                   /* (o) Number of bytes encoded */ 
    speex_Dec_Inst_t *global_vars   /* (i/o) Encoder instance */ 
)
{
    speex_bits_destroy(&global_vars->bits);
    speex_decoder_destroy(global_vars->state);

}

int speex_decode (
	speex_Dec_Inst_t *global_vars,
    short *decoded_data,			/* output */
    short *encoded_data,				/* input */
	int lg
)
{

	float output[FRAME_SIZE];
	char cbits[200];
	short * ifc;
	char * ifc2;
	int i;
	ifc2 = (char*) encoded_data;
	for (i=0; i<lg;i++) {
		cbits [i] = *ifc2;
		ifc2++;
		}


	int nbBytes =lg;
	/*Copy the data into the bit-stream struct*/
	speex_bits_read_from(&global_vars->bits, cbits, nbBytes);
//	/*Decode the data*/
//	speex_decode(global_vars->state, &global_vars->bits, output);
//	ifc = decoded_data;
//	for (i=0; i<FRAME_SIZE;i++) {
//		*ifc = output [i];
//		ifc++;
//		}
//	return FRAME_SIZE;

	/*Decode the data*/
	ifc = decoded_data;
	while (speex_bits_remaining(&global_vars->bits) >= MIN_SPEEX_FRAME_SIZE) {
		if (speex_decode(global_vars->state, &global_vars->bits, output) < 0)
		return 0; 
		for (i=0; i<FRAME_SIZE;i++) {
			*ifc = (short)output [i];
			ifc++;
		}
	}
return ifc - decoded_data;
}
