#ifndef CALLAUDIOJACKOUT_H_INCLUDED
#define CALLAUDIOJACKOUT_H_INCLUDED

#include <jack/jack.h>


class DspAudioJackOut
{
public:
	DspAudioJackOut( DspOutRtp *inrtp, const QString &client = "kphone" );

	/**
	 * Destructor.  Will close the device if it is open.
	 */
	virtual ~DspAudioJackOut( void );

	jack_port_t *input_port;
	jack_port_t *output_port;
	void setCodec( const codecType newCodec, int newCodecNum );
	void start( void );
	void exit( void );
	bool ok( void ) { return jack_client != 0; }

private:
	void mono_to_stereo(void *b_from, void *b_to, size_t count);
	void stereo_to_mono(void *b_from, void *b_to, size_t count);
	bool audio_stereo;
//	AudioBuffer output_buf;   // Output buffer
	
//	DspOut *input;

/* JACK data */
	jack_client_t *jack_client;
	QString jack_clientName;
//	jack_thread_info_t thread_info;

};

#endif  // DSPOUTJACK_H_INCLUDED
