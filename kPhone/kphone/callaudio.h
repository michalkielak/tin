/*
 * Copyright (c) 2000 Billy Biggs <bbiggs@div8.net>
 * Copyright (c) 2004 Wirlab <kphone@wirlab.net>
 * Copyright (c) 2006 Kphone Team  <kphone-devel@lists.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.
 * 
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 *
 */
#ifndef CALLAUDIO_H_INCLUDED
#define CALLAUDIO_H_INCLUDED

#include <qobject.h>
#include <qstring.h>
#include <sys/types.h>

#include "../dissipate2/udpmessagesocket.h"
#include "../dissipate2/sipcall.h"
#include "sessioncontrol.h"

#include "../kaudio/dspaudioout.h"
#include "../kaudio/dspaudioin.h"
#include "../kaudio/dspoutrtp.h"
#include "../kaudio/dspoutoss.h"
#include "../kaudio/dspoutalsa.h"
#include "../kaudio/dspouttone.h"

#include "../dissipate2/sdpbuild.h"
class DspAudio;
class DspOutRtp;
/**
@short This Class controls the noise
*
* CallAudio allocates and starts the audio related RTP and sound resources
*
* The class is used by KCallWidget
*/
class CallAudio : public QObject
{
Q_OBJECT
public:
/**
*	the constructor
*/
    CallAudio(SessionControl *sessionC, const char *name );

    ~CallAudio();
/**
*	
*/
//	bool isOutputOSS( void ) const { return useoss; }
/**
*	
*/
//	void useOSS( void );

	// Call attachment
/**
*	stop Listening to Audio
*/	
        void stopListeningAudio( int aU );

/**
*	stop listening and sending audio 
*/	
	void stopAudio( int aU );

/**
*	stop sending audio via RTP
*/	
	void stopRTPAudio( int aU );

//	void stopEarlyAudio( int aU );


/**
*	begin to send a DTMF tone
*/	
	void startDTMF(int tone); // Start DTMF tone generation

/**
*	send a DTMF Sequence
*/	
	void startDTMFSeq(QString string, int size); // Start DTMF tone generation

/**
*	stop to send a DTMF tone
*/	
	void stopDTMF(void);  // Stop DTMF tone generation

/**
*	allocate outgoing audio related RTP resources
*/	
	int audioOut(  SdpBuild *call,int aU );

/**
*	allocate incoming audio related RTP resources,
*	attach audio to the audio card and start transmission
*/	
	int  audioIn(  SdpBuild *call, bool tone, int aU );
	
/**
*	switch between tone and alsa
*/	
	void audioTone( bool tone, int aU );
	
// Settings
/**
*	
*/
	bool isAudioOn( void );
/**
*	
*/
	void ringOnce(int mul);
/**
*	
*/	
	void clearMe(int cwuser);
/**
*	
*/	
	bool audioOwner(int cwuser);
/**
*	
*/
	UDPMessageSocket *getSocket(void) {return &socket;}
signals:
private slots:
/**
* emit a sequence of DTMF chars	
*/	void handleDTMFSeq(void);

private:
	SessionControl *sc;
	DspOutRtp  *outrtp;
	DspOutRtp  *inrtp;
	DspOutOss  *inoss;
	DspOutAlsa *inalsa;
	DspOutTone *intone;
	DspOutOss  *outoss; 
	DspOutAlsa *outalsa; 
	DspAudioOut	*output;
	DspAudioIn	*input; 
	int audio_fd;
	bool useoss;
	QString ossfilename;
	QString ossfilename2;
	int payload;
	bool useStun;
	bool symMedia;
	char charDTMF[12];
	QTimer *dtmfTimer;
	int dtmfCount;
	int dtmfSize;
	QString dtmfString;
	/*
	 * Common socket for both streams, used only in symmetric mode
	 */
	UDPMessageSocket socket;
	int debug;
	int audioUser;
	bool outRunning;
};


#endif //CALLAUDIO_H_INCLUDED
