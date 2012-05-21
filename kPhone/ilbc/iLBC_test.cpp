
   /******************************************************************

       iLBC Speech Coder ANSI-C Source Code

       iLBC_test.c

       Copyright (C) The Internet Society (2004).
       All Rights Reserved.

   ******************************************************************/

   #include <math.h>
   #include <stdlib.h>
   #include <stdio.h>
   #include <string.h>
   #include "iLBC_define.h"
   #include "iLBC_encode.h"
   #include "iLBC_decode.h"

   /* Runtime statistics */
   #include <time.h>

   #define ILBCNOOFWORDS_MAX   (NO_OF_BYTES_30MS/2)

   /*----------------------------------------------------------------*
    *  Encoder interface function





    *---------------------------------------------------------------*/

   short encode(   /* (o) Number of bytes encoded */
       iLBC_Enc_Inst_t *iLBCenc_inst,
                                   /* (i/o) Encoder instance */
       short *encoded_data,    /* (o) The encoded bytes */
       short *data                 /* (i) The signal block to encode*/
   ){
       float block[BLOCKL_MAX];
       int k;

       /* convert signal to float */

       for (k=0; k<iLBCenc_inst->blockl; k++)
           block[k] = (float)data[k];

       /* do the actual encoding */

       iLBC_encode((unsigned char *)encoded_data, block, iLBCenc_inst);


       return (iLBCenc_inst->no_of_bytes);
   }

   /*----------------------------------------------------------------*
    *  Decoder interface function
    *---------------------------------------------------------------*/

   short decode(       /* (o) Number of decoded samples */
       iLBC_Dec_Inst_t *iLBCdec_inst,  /* (i/o) Decoder instance */
       short *decoded_data,        /* (o) Decoded signal block*/
       short *encoded_data,        /* (i) Encoded bytes */
       short mode                       /* (i) 0=PL, 1=Normal */
   ){
       int k;
       float decblock[BLOCKL_MAX], dtmp;

       /* check if mode is valid */

       if (mode<0 || mode>1) {
           printf("\nERROR - Wrong mode - 0, 1 allowed\n"); exit(3);}

       /* do actual decoding of block */

       iLBC_decode(decblock, (unsigned char *)encoded_data,
           iLBCdec_inst, mode);

       /* convert to short */





       for (k=0; k<iLBCdec_inst->blockl; k++){
           dtmp=decblock[k];

           if (dtmp<MIN_SAMPLE)
               dtmp=MIN_SAMPLE;
           else if (dtmp>MAX_SAMPLE)
               dtmp=MAX_SAMPLE;
           decoded_data[k] = (short) dtmp;
       }

       return (iLBCdec_inst->blockl);
   }

   /*---------------------------------------------------------------*
    *  Main program to test iLBC encoding and decoding
    *
    *  Usage:
    *    exefile_name.exe <infile> <bytefile> <outfile> <channel>
    *
    *    <infile>   : Input file, speech for encoder (16-bit pcm file)
    *    <bytefile> : Bit stream output from the encoder
    *    <outfile>  : Output file, decoded speech (16-bit pcm file)
    *    <channel>  : Bit error file, optional (16-bit)
    *                     1 - Packet received correctly
    *                     0 - Packet Lost
    *
    *--------------------------------------------------------------*/

   int main(int argc, char* argv[])
   {

       /* Runtime statistics */

       float starttime;
       float runtime;
       float outtime;

       FILE *ifileid,*efileid,*ofileid, *cfileid;
       short data[BLOCKL_MAX];
       short encoded_data[ILBCNOOFWORDS_MAX], decoded_data[BLOCKL_MAX];
       int len;
       short pli, mode;
       int blockcount = 0;
       int packetlosscount = 0;

       /* Create structs */
       iLBC_Enc_Inst_t Enc_Inst;
       iLBC_Dec_Inst_t Dec_Inst;





       /* get arguments and open files */

       if ((argc!=5) && (argc!=6)) {
           printf(
           "\n*-----------------------------------------------*\n");
           printf(
           "   %s <20,30> input encoded decoded (channel)\n\n",
               argv[0]);
           printf(
           "   mode    : Frame size for the encoding/decoding\n");
           printf(
           "                 20 - 20 ms\n");
           printf(
           "                 30 - 30 ms\n");
           printf(
           "   input   : Speech for encoder (16-bit pcm file)\n");
           printf(
           "   encoded : Encoded bit stream\n");
           printf(
           "   decoded : Decoded speech (16-bit pcm file)\n");
           printf(
           "   channel : Packet loss pattern, optional (16-bit)\n");
           printf(
           "                  1 - Packet received correctly\n");
           printf(
           "                  0 - Packet Lost\n");
           printf(
           "*-----------------------------------------------*\n\n");
           exit(1);
       }
       mode=atoi(argv[1]);
       if (mode != 20 && mode != 30) {
           printf("Wrong mode %s, must be 20, or 30\n",
               argv[1]);
           exit(2);
       }
       if ( (ifileid=fopen(argv[2],"rb")) == NULL) {
           printf("Cannot open input file %s\n", argv[2]);
           exit(2);}
       if ( (efileid=fopen(argv[3],"wb")) == NULL) {
           printf( "Cannot open encoded file %s\n",
               argv[3]); exit(1);}
       if ( (ofileid=fopen(argv[4],"wb")) == NULL) {
           printf( "Cannot open decoded file %s\n",
               argv[4]); exit(1);}
       if (argc==6) {
           if( (cfileid=fopen(argv[5],"rb")) == NULL) {
               printf( "Cannot open channel file %s\n",





                   argv[5]);
               exit(1);
           }
       } else {
           cfileid=NULL;
       }

       /* print info */

       printf( "\n");
       printf(
           "*---------------------------------------------------*\n");
       printf(
           "*                                                   *\n");
       printf(
           "*      iLBC test program                            *\n");
       printf(
           "*                                                   *\n");
       printf(
           "*                                                   *\n");
       printf(
           "*---------------------------------------------------*\n");
       printf("\nMode           : %2d ms\n", mode);
       printf("Input file     : %s\n", argv[2]);
       printf("Encoded file   : %s\n", argv[3]);
       printf("Output file    : %s\n", argv[4]);
       if (argc==6) {
           printf("Channel file   : %s\n", argv[5]);
       }
       printf("\n");

       /* Initialization */

       initEncode(&Enc_Inst, mode);
       initDecode(&Dec_Inst, mode, 1);

       /* Runtime statistics */

       starttime=clock()/(float)CLOCKS_PER_SEC;

       /* loop over input blocks */

       while (fread(data,sizeof(short),Enc_Inst.blockl,ifileid)==
               Enc_Inst.blockl) {

           blockcount++;

           /* encoding */





           printf( "--- Encoding block %i --- ",blockcount);
           len=encode(&Enc_Inst, encoded_data, data);
           printf( "\r");

           /* write byte file */

           fwrite(encoded_data, sizeof(unsigned char), len, efileid);

           /* get channel data if provided */
           if (argc==6) {
               if (fread(&pli, sizeof(short), 1, cfileid)) {
                   if ((pli!=0)&&(pli!=1)) {
                       printf( "Error in channel file\n");
                       exit(0);
                   }
                   if (pli==0) {
                       /* Packet loss -> remove info from frame */
                       memset(encoded_data, 0,
                           sizeof(short)*ILBCNOOFWORDS_MAX);
                       packetlosscount++;
                   }
               } else {
                   printf( "Error. Channel file too short\n");
                   exit(0);
               }
           } else {
               pli=1;
           }

           /* decoding */

           printf( "--- Decoding block %i --- ",blockcount);

           len=decode(&Dec_Inst, decoded_data, encoded_data, pli);
           printf( "\r");

           /* write output file */

           fwrite(decoded_data,sizeof(short),len,ofileid);
       }

       /* Runtime statistics */

       runtime = (float)(clock()/(float)CLOCKS_PER_SEC-starttime);
       outtime = (float)((float)blockcount*(float)mode/1000.0);
       printf("\n\nLength of speech file: %.1f s\n", outtime);
       printf("Packet loss          : %.1f%%\n",
           100.0*(float)packetlosscount/(float)blockcount);





       printf("Time to run iLBC     :");
       printf(" %.1f s (%.1f %% of realtime)\n\n", runtime,
           (100*runtime/outtime));

       /* close files */

       fclose(ifileid);  fclose(efileid); fclose(ofileid);
       if (argc==6) {
           fclose(cfileid);
       }
       return(0);
   }

