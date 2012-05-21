#ifndef CALLAUDIOJACKIN_H_INCLUDED
#define CALLAUDIOJACKIN_H_INCLUDED

#include <jack/jack.h>

class DspAudioJackIn
{
public:
	DspAudioJackIn( DspOutRtp *inrtp, const QString &client = "kphone" );

	/**
	 * Destructor.  Will close the device if it is open.
	 */
	virtual ~DspAudioJackIn( void );

	jack_port_t *input_port;
	jack_port_t *output_port;
	void start( void );
	void exit( void );


private:
	void mono_to_stereo(void *b_from, void *b_to, size_t count);
	void stereo_to_mono(void *b_from, void *b_to, size_t count);
	bool audio_stereo;
//	AudioBuffer output_buf;   // Output buffer


/* JACK data */
	jack_client_t *jack_client;
	QString jack_clientName;
//	jack_thread_info_t thread_info;

};

#endif  // DSPOUTJACK_H_INCLUDED
