#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <sys/types.h>


int ringCount;
int audio_fd;
unsigned char *ringtonebuffer;
long int ringtonedatasize;
FILE *ringtonewavefile;
typedef  unsigned int uint32_t;
typedef  unsigned short uint16_t;

struct w {
		char fourcc_riff[4];         /* "RIFF" */
		uint32_t file_size;
		char fourcc_wave[4];         /* "WAVE" */
		char fourcc_format[4];       /* "fmt " */
		uint32_t formatheadersize;
		uint16_t audio_format;
		uint16_t num_channels;
		uint32_t audio_rate;
		uint32_t bytes_per_second;
		uint16_t block_align;
		uint16_t sample_size;
		char fourcc_data[4];         /* "data" */
		uint32_t data_size;
} waveheader;

const char *filename = "default";
const char *rdevice  = "/dev/dsp";
int select_m = 1;
int i;
int main (int argc, char **argv) {


if (argc == 2) {
    filename  = *(argv + 1);
    if (filename != "default") {
	select_m = 0;
	printf ("we play %s on /dev/dsp\n", rdevice);
    } else {
	printf ("beeping on /dev/dsp\n");
    }
} else if(argc == 3) {
    rdevice  = *(argv + 2);
    filename = *(argv + 1);
    if (filename != "default") {
	select_m = 0;
	printf ("we play %s on %s\n", filename, rdevice);
    } else {
	printf ("beeping on %s\n", rdevice);
    }
}   else {
    printf ("beeping on /dev/dsp\n");
}


if (filename != "default") select_m = 0;

while(1) {
	if(select_m) {
		audio_fd = open( rdevice , O_WRONLY | O_NONBLOCK );
		if( audio_fd == -1 ) {
			printf( "ERROR: %s\n", "Open Failed" );
			exit(1);
		}
		int flags = fcntl( audio_fd, F_GETFL );
		flags &= ~O_NONBLOCK;
		fcntl( audio_fd, F_SETFL, flags );
		int format = AFMT_S16_LE;
		if( ioctl( audio_fd, SNDCTL_DSP_SETFMT, &format ) == -1 ) {
			exit(1);
		}
		if( format != AFMT_S16_LE ) {
			exit(1);
		}
		int channels = 1;
		if( ioctl( audio_fd, SNDCTL_DSP_CHANNELS, &channels ) == -1 ) {
			exit(1);
		}
		if( channels != 1 ) {
			exit(1);
		}
		int rate = 8000;
		if( ioctl( audio_fd, SNDCTL_DSP_SPEED, &rate ) == -1 ) {
			exit(1);
		}
		if( rate != 8000) {
			exit(1);
		}
		int size = 4096;
		int samp = size/2;
		int ampl = 16384;
		unsigned char devbuf[size];
		int buf[samp];
		int p=0;
		double arg1, arg2;
		arg1 = (double)2 * (double)3.1416 * (double)941 / (double)samp;
		arg2 = (double)2 * (double)3.1416 * (double)1336 / (double)samp;
		for(i = 0; i < samp; i++) {
			buf[i] = (short)((double)(ampl) * sin(arg1 * i) +
				(double)(ampl) * sin(arg2 * i));
		};
		for (i=0; i<samp; i+=2) {
			devbuf[p++] = (unsigned char)(buf[i] & 0xff);
			devbuf[p++] = (unsigned char)((buf[i] >> 8) & 0xff);
		}
		for(;;) {
			if( write( audio_fd, devbuf, samp ) != -1 ) {
				break;
			}
		}
		close( audio_fd );
	} else {
	
		printf ("Playing ringtone from file \"%s\"\n", filename);
		ringtonewavefile = fopen (filename, "r");
		if (ringtonewavefile == NULL) {
			printf ("ERROR:  Open failed for ringtone wave file\n");
			exit(1);
		}
		fread (&waveheader, 44, 1, ringtonewavefile);
		if ((waveheader.fourcc_riff[0] != 'R') ||
				(waveheader.fourcc_riff[1] != 'I') ||
				(waveheader.fourcc_riff[2] != 'F') ||
				(waveheader.fourcc_riff[3] != 'F')) {
			printf ("Ringtone file is not in wave format, so refusing to use it\n");
			fclose (ringtonewavefile);
			exit(1);
		} else {
			if (waveheader.audio_format != 1) {
				printf ("Ringtone file is not in WAVE/PCM format, so refusing to use it\n");
				fclose (ringtonewavefile);
				exit(1);
			}
			if ((waveheader.fourcc_format[0] != 'f') ||
					(waveheader.fourcc_format[1] != 'm') ||
					(waveheader.fourcc_format[2] != 't') ||
					(waveheader.fourcc_format[3] != ' ')) {
				printf ("Ringtone file's format chunk not where expected, so refusing to use it\n");
				fclose (ringtonewavefile);
				exit(1);
			}
			if ((waveheader.fourcc_data[0] != 'd') ||
					(waveheader.fourcc_data[1] != 'a') ||
					(waveheader.fourcc_data[2] != 't') ||
					(waveheader.fourcc_data[3] != 'a')) {
				printf ("Ringtone file's data chunk not where expected, so refusing to use it\n");
				fclose (ringtonewavefile);
				exit(1);
			}
		}
		ringtonebuffer = (unsigned char*) malloc (waveheader.data_size);
		ringtonedatasize = fread (ringtonebuffer, 1, waveheader.data_size, ringtonewavefile);
		fclose (ringtonewavefile);
		audio_fd = open( rdevice, O_WRONLY | O_NONBLOCK );
		if( audio_fd == -1 ) {
			printf( "ERROR:  Open failed for ringtone audio device\n" );
			exit(1);
		}
		int flags = fcntl( audio_fd, F_GETFL );
		flags &= ~O_NONBLOCK;
		fcntl( audio_fd, F_SETFL, flags );
		int format;
		if (waveheader.sample_size == 16) {
			format = AFMT_S16_LE;
		} else {
			format = AFMT_U8;
		}
		int requestedformat = format;
		if( ioctl( audio_fd, SNDCTL_DSP_SETFMT, &format ) == -1 ) {
			printf ("ERROR:  Unable to set audio format for ringtone device\n");
			exit(1);
		}
		if( format != requestedformat ) {
			printf ("ERROR:  Unable to set audio format for ringtone device\n");
			exit(1);
		}
		int channels = waveheader.num_channels;
		if( ioctl( audio_fd, SNDCTL_DSP_CHANNELS, &channels ) == -1 ) {
			printf ("ERROR:  Unable to set number of audio channels for ringtone device\n");
			exit(1);
		}
		if( channels != waveheader.num_channels ) {
			printf ("ERROR:  Unable to set number of audio channels for ringtone device\n");
			exit(1);
		}
		unsigned int rate = waveheader.audio_rate;
		if( ioctl( audio_fd, SNDCTL_DSP_SPEED, &rate ) == -1 ) {
			printf ("ERROR:  Unable to set audio rate for ringtone device\n");
			exit(1);
		}
		if( rate != waveheader.audio_rate) {
			printf ("ERROR:  Unable to set audio rate for ringtone device\n");
			exit(1);
		}
		write (audio_fd, ringtonebuffer, ringtonedatasize);
		free (ringtonebuffer);
		ioctl (audio_fd, SNDCTL_DSP_POST, 0);
		ioctl (audio_fd, SNDCTL_DSP_SYNC, 0);
		close( audio_fd );
	}
	sleep (1);
   }
}
