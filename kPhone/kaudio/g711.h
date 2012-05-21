#ifndef G711_H_INCLUDED
#define G711_H_INCLUDED

unsigned char linear2ulaw( short sample );
short ulaw2linear( unsigned char ulawbyte );

unsigned char linear2alaw( short sample );
short alaw2linear( unsigned char alawbyte );

#endif // G711_H_INCLUDED
