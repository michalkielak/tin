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
#include "sdpbuild.h"
#include "../kphone/sessioncontrol.h"
#include "../kphone/kstatics.h"

SdpBuild::SdpBuild ( SessionControl *sco, SipCall *scall)
{
	tempresponsepay=160;
	sendrec=false;
	msg="=";
	call=scall;
	sessionC=sco;
	testPort = "0";
	testHomePort = "0";
	audioPort = "0";
	audioHomePort = "0";
	responsePay = 160;
	videoPrefCodec = " ";
	prefCodec = " ";
	vecID = " ";
	cntParam = 0;
	remoteStart = false;
	if (sessionC) {
		if (sessionC->getSrtpMode() >0) {	
		trsp   = " RTP/SAVP ";
		} else {
		trsp   = " RTP/AVP ";	
		}
	}
	auEnd  = "8000\r\n";
	viEnd  = "90000\r\n";
	end    = "\r\n";
	telend = "101\r\n";
	mIs    = "m=application";
	mAudio = "m=audio ";
	mVideo = "m=video ";
	aRMap  = "a=rtpmap:";
	tEvent = "101 telephone-event";
	ReInvite = false;
	if(KStatics::haveIPv6){
		testHost = "::0";
	} else { 
		testHost = "0.0.0.0";
	}
	if(sessionC) {
		responsePay = sessionC->getPayload();	
	}
}

SdpBuild::~SdpBuild(void)
{

}
QString SdpBuild::prepInviteString(int load,QString audioport,QString port) {


// Evaluate the media type for the SDP-description
        msg = "v=0\r\n"; // Version
        msg += "o=username 0 0 IN ";
	if(KStatics::haveIPv6) msg+="IP6 "; else msg+="IP4 ";
	msg += Sip::getLocalAddress() +end; // Session Id

        msg += "s=The Funky Flow\r\n"; // Session Name
        msg += "c=IN ";
	if(KStatics::haveIPv6) msg+="IP6 "; else msg+="IP4 ";
	msg += Sip::getLocalAddress() +end; // Connection Info
        msg += "t=0 0\r\n"; // Time active

// prepare the Media information

    if (load == (int)SipCall::StandardCall) {
 	msg += mAudio + audioport  + trsp + sessionC->getNZPrefCodecNum(0) + sessionC->getNZPrefCodecNum(1)+
	+ sessionC->getNZPrefCodecNum(2) + sessionC->getNZPrefCodecNum(3) + sessionC->getNZPrefCodecNum(4) + telend; 


	if(sessionC->getPrefCodecNum(0) != "-1") msg += aRMap + sessionC->getPrefCodecNum(0) + " " + sessionC->getPrefCodec(0) + "/" + auEnd;
	if(sessionC->getPrefCodecNum(1) != "-1") msg += aRMap + sessionC->getPrefCodecNum(1) + " " + sessionC->getPrefCodec(1) + "/" + auEnd;
	if(sessionC->getPrefCodecNum(2) != "-1") msg += aRMap + sessionC->getPrefCodecNum(2) + " " + sessionC->getPrefCodec(2) + "/" + auEnd;
	if(sessionC->getPrefCodecNum(3) != "-1") msg += aRMap + sessionC->getPrefCodecNum(3) + " " + sessionC->getPrefCodec(3) + "/" + auEnd;
	if(sessionC->getPrefCodecNum(4) != "-1") msg += aRMap + sessionC->getPrefCodecNum(4) + " " + sessionC->getPrefCodec(4) + "/" + auEnd;		
	msg += aRMap + tEvent + "/" + auEnd;
	
	if(sessionC->getILBC()) {
		if(getResponsePay() == 240) {
			msg += "a=fmtp:97 mode=30\r\n";
		} else {
			msg += "a=fmtp:97 mode=20\r\n";
		}
	}
    } else if( load == (int)SipCall::videoCall) {
     
	msg += mVideo+port+trsp+ sessionC-> getNZVidPrefCodecNum (0)+ " " + sessionC->getNZVidPrefCodecNum (1)+
		" " + sessionC->getNZVidPrefCodecNum (2)+end;	
		
	if(sessionC->getVidPrefCodecNum(0) != "-1") msg += aRMap + sessionC->getVidPrefCodecNum(0) + " " + sessionC->getVidPrefCodec(0) + "/" + viEnd;
	if(sessionC->getVidPrefCodecNum(1) != "-1") msg += aRMap + sessionC->getVidPrefCodecNum(1) + " " + sessionC->getVidPrefCodec(1) + "/" + viEnd;
	if(sessionC->getVidPrefCodecNum(2) != "-1") msg += aRMap + sessionC->getVidPrefCodecNum(2) + " " + sessionC->getVidPrefCodec(2) + "/" + viEnd;
	
    } else if( load == (int)SipCall::auviCall) {
   	
	msg += mAudio + audioport  + trsp + sessionC->getNZPrefCodecNum(0) + sessionC->getNZPrefCodecNum(1)+
	+ sessionC->getNZPrefCodecNum(2) + sessionC->getNZPrefCodecNum(3) + sessionC->getNZPrefCodecNum(4) + telend; 
	if(sessionC->getPrefCodecNum(0) != "-1") msg += aRMap + sessionC->getPrefCodecNum(0) + " " + sessionC->getPrefCodec(0) + "/" + auEnd;
	if(sessionC->getPrefCodecNum(1) != "-1") msg += aRMap + sessionC->getPrefCodecNum(1) + " " + sessionC->getPrefCodec(1) + "/" + auEnd;
	if(sessionC->getPrefCodecNum(2) != "-1") msg += aRMap + sessionC->getPrefCodecNum(2) + " " + sessionC->getPrefCodec(2) + "/" + auEnd;
	if(sessionC->getPrefCodecNum(3) != "-1") msg += aRMap + sessionC->getPrefCodecNum(3) + " " + sessionC->getPrefCodec(3) + "/" + auEnd;
	if(sessionC->getPrefCodecNum(4) != "-1") msg += aRMap + sessionC->getPrefCodecNum(4) + " " + sessionC->getPrefCodec(4) + "/" + auEnd;		
	msg += aRMap + tEvent + "/" + auEnd;
	
	if(sessionC->getILBC()) {
		if(getResponsePay() == 240) {
			msg += "a=fmtp:97 mode=30\r\n";
		} else {
			msg += "a=fmtp:97 mode=20\r\n";
		}
	}
	msg += mVideo+port+trsp+ sessionC-> getNZVidPrefCodecNum (0)+ " " + sessionC->getNZVidPrefCodecNum (1)+
		" " + sessionC->getNZVidPrefCodecNum (2)+end;	
	if(sessionC->getVidPrefCodecNum(0) != "-1") msg += aRMap + sessionC->getVidPrefCodecNum(0) + " " + sessionC->getVidPrefCodec(0) + "/" + viEnd;
	if(sessionC->getVidPrefCodecNum(1) != "-1") msg += aRMap + sessionC->getVidPrefCodecNum(1) + " " + sessionC->getVidPrefCodec(1) + "/" + viEnd;
	if(sessionC->getVidPrefCodecNum(2) != "-1") msg += aRMap + sessionC->getVidPrefCodecNum(2) + " " + sessionC->getVidPrefCodec(2) + "/" + viEnd;
	    
} else {
	
	int posN=sessionC->theVectorId(load).find( '\n' ) + 1;
        vecID = sessionC->theVectorId(load);
	vecID = vecID.remove( 0, posN );
	int trans=sessionC->useL4(load);
	if(trans==useudpL4){
			transp=" udp ";
	} else {
			transp=" RTP/AVP ";
	}
	msg += mIs +  " " +  port  + transp + vecID + end;
	if(call->isRemoteStart()) {
	            msg += "a=fmtp:" + vecID + " rstart" + end;
        }
}
    	msg += "a=sendrecv\r\n";     //  Full duplex call

return msg;
}




QString SdpBuild::prepAcceptString(int load,QString audioport,QString port, bool isHold) {

// Evaluate the media type for the SDP-description
	msg = "v=0\r\n"; // Version
        msg += "o=username 0 0 IN ";
	if(KStatics::haveIPv6) msg+="IP6 "; else msg+="IP4 ";	// Connection Info
	msg += Sip::getLocalAddress() +end;// Session Id
        
	if(isHold) {
        	msg += "s=Put me on Hold\r\n";  // Session Name
		msg += "c=IN ";
		if(KStatics::haveIPv6) msg+="IP6 ::0\r\n"; else msg+="IP4 0.0.0.0\r\n";	// Connection Info
	} else {
		msg += "s=The Funky Flow\r\n"; // Session Name
		msg += "c=IN ";
		if(KStatics::haveIPv6) msg+="IP6 "; else msg+="IP4 ";
		msg += Sip::getLocalAddress() +end; // Connection Info
	}
        msg += "t=0 0\r\n"; // Time active

// prepare the Media information

if (load == SipCall::StandardCall) {
	
	
	if(getPrefCodecNum() == sessionC->getPrefCodecNum(0) ) {
           msg += mAudio + audioport  + trsp + sessionC->getNZPrefCodecNum(0) + sessionC->getNZPrefCodecNum(1)+
	+ sessionC->getNZPrefCodecNum(2) + sessionC->getNZPrefCodecNum(3) + sessionC->getNZPrefCodecNum(4) + telend;
    
	} else if(getPrefCodecNum() == sessionC->getPrefCodecNum(1) ) {
           msg += mAudio + audioport + trsp + sessionC->getNZPrefCodecNum(1)+ sessionC->getNZPrefCodecNum(0)+
	+ sessionC->getNZPrefCodecNum(2) + sessionC->getNZPrefCodecNum(3) + sessionC->getNZPrefCodecNum(4) + telend;

	} else if(getPrefCodecNum() == sessionC->getPrefCodecNum(2) ){
           msg += mAudio + audioport  + trsp + sessionC->getNZPrefCodecNum(2)+ sessionC->getNZPrefCodecNum(0) +
	+ sessionC->getNZPrefCodecNum(1)+ sessionC->getNZPrefCodecNum(3)+ sessionC->getNZPrefCodecNum(4) + telend;

	} else if(getPrefCodecNum() == sessionC->getPrefCodecNum(3) ){
           msg += mAudio + audioport  + trsp + sessionC->getNZPrefCodecNum(3)+ sessionC->getNZPrefCodecNum(0) +
	sessionC->getNZPrefCodecNum(1)+ sessionC->getNZPrefCodecNum(2)+ sessionC->getNZPrefCodecNum(4) + telend;

	} else if(getPrefCodecNum() == sessionC->getPrefCodecNum(4) )        {
          msg += mAudio + audioport  +trsp+ sessionC-> getNZPrefCodecNum(4)+ " " + sessionC->getNZPrefCodecNum(0) +
	" " + sessionC->getNZPrefCodecNum(1)+ + sessionC->getNZPrefCodecNum(2) + sessionC->getNZPrefCodecNum(3) + telend;
        } else {
	   msg += mAudio + audioport  + trsp + sessionC->getNZPrefCodecNum(0) + sessionC->getNZPrefCodecNum(1)+
	+ sessionC->getNZPrefCodecNum(2) + sessionC->getNZPrefCodecNum(3) + sessionC->getNZPrefCodecNum(4) + telend;
	}
	if(sessionC->getPrefCodecNum(0) != "-1") msg += aRMap + sessionC->getPrefCodecNum(0) + " " + sessionC->getPrefCodec(0) + "/" + auEnd;
	if(sessionC->getPrefCodecNum(1) != "-1") msg += aRMap + sessionC->getPrefCodecNum(1) + " " + sessionC->getPrefCodec(1) + "/" + auEnd;
	if(sessionC->getPrefCodecNum(2) != "-1") msg += aRMap + sessionC->getPrefCodecNum(2) + " " + sessionC->getPrefCodec(2) + "/" + auEnd;
	if(sessionC->getPrefCodecNum(3) != "-1") msg += aRMap + sessionC->getPrefCodecNum(3) + " " + sessionC->getPrefCodec(3) + "/" + auEnd;
	if(sessionC->getPrefCodecNum(4) != "-1") msg += aRMap + sessionC->getPrefCodecNum(4) + " " + sessionC->getPrefCodec(4) + "/" + auEnd;
	msg += aRMap + tEvent + "/" + auEnd;
	
	if(sessionC->getILBC()) {
		if(getResponsePay() == 240) {
			msg += "a=fmtp:97 mode=30\r\n";
		} else {
			msg += "a=fmtp:97 mode=20\r\n";
		}
	}

} else if( load == SipCall::videoCall) {
      	if (sessionC->getVidPrefCodec(0) == getVidPrefCodec()) {
        	msg += mVideo+port+trsp+ sessionC-> getNZVidPrefCodecNum (0)+ " " + sessionC->getNZVidPrefCodecNum (1)+
		" " + sessionC->getVidPrefCodecNum (2)+end;
        } else if (sessionC->getVidPrefCodec(1)== getVidPrefCodec() ) {
		msg += mVideo+port+trsp+ sessionC-> getNZVidPrefCodecNum (1)+ " " + sessionC->getNZVidPrefCodecNum (0)+
		" " + sessionC->getVidPrefCodecNum (2)+end;
	} else         {
        	msg += mVideo+port+trsp+ sessionC-> getNZVidPrefCodecNum (2)+ " " + sessionC->getNZVidPrefCodecNum (0)+
		" " + sessionC->getNZVidPrefCodecNum (1)+end;
        }
	if(sessionC->getVidPrefCodecNum(0) != "-1") msg += aRMap + sessionC->getVidPrefCodecNum(0) + " " + sessionC->getVidPrefCodec(0) + "/" + viEnd;
	if(sessionC->getVidPrefCodecNum(1) != "-1") msg += aRMap + sessionC->getVidPrefCodecNum(1) + " " + sessionC->getVidPrefCodec(1) + "/" + viEnd;
	if(sessionC->getVidPrefCodecNum(2) != "-1") msg += aRMap + sessionC->getVidPrefCodecNum(2) + " " + sessionC->getVidPrefCodec(2) + "/" + viEnd;

} else if( load == SipCall::auviCall) {


 	if(getPrefCodec().contains(sessionC->getPrefCodec(0))){
           msg += mAudio + audioport  + trsp + sessionC->getNZPrefCodecNum(0) + sessionC->getNZPrefCodecNum(1)+
	" " + sessionC->getNZPrefCodecNum(2) + sessionC->getNZPrefCodecNum(3) + sessionC->getNZPrefCodecNum(4) + telend;

	} else if(getPrefCodec().contains(sessionC->getPrefCodec(1))){
           msg += mAudio + audioport + trsp + sessionC->getNZPrefCodecNum(1)+ sessionC->getNZPrefCodecNum(0)+
	" " + sessionC->getNZPrefCodecNum(2) + sessionC->getNZPrefCodecNum(3) + sessionC->getNZPrefCodecNum(4) + telend;

	} else if(getPrefCodec().contains(sessionC->getPrefCodec(2))){
           msg += mAudio + audioport  + trsp + sessionC->getNZPrefCodecNum(2)+ sessionC->getNZPrefCodecNum(0) +
	+ sessionC->getNZPrefCodecNum(1)+ sessionC->getNZPrefCodecNum(3)+ sessionC->getNZPrefCodecNum(4) + telend;

	} else if(getPrefCodec().contains(sessionC->getPrefCodec(3))){
           msg += mAudio + audioport  + trsp + sessionC->getNZPrefCodecNum(3)+ sessionC->getNZPrefCodecNum(0) +
	sessionC->getNZPrefCodecNum(1)+ sessionC->getNZPrefCodecNum(2)+ sessionC->getNZPrefCodecNum(4) + telend;

	} else         {
          msg += mAudio + audioport  +trsp+ sessionC-> getNZPrefCodecNum(4)+ " " + sessionC->getNZPrefCodecNum(0) +
	" " + sessionC->getNZPrefCodecNum(1)+ + sessionC->getNZPrefCodecNum(2) + sessionC->getNZPrefCodecNum(3) + telend;
        }
	if(sessionC->getPrefCodecNum(0) != "-1") msg += aRMap + sessionC->getPrefCodecNum(0) + " " + sessionC->getPrefCodec(0) + "/" + auEnd;
	if(sessionC->getPrefCodecNum(1) != "-1") msg += aRMap + sessionC->getPrefCodecNum(1) + " " + sessionC->getPrefCodec(1) + "/" + auEnd;
	if(sessionC->getPrefCodecNum(2) != "-1") msg += aRMap + sessionC->getPrefCodecNum(2) + " " + sessionC->getPrefCodec(2) + "/" + auEnd;
	if(sessionC->getPrefCodecNum(3) != "-1") msg += aRMap + sessionC->getPrefCodecNum(3) + " " + sessionC->getPrefCodec(3) + "/" + auEnd;
	if(sessionC->getPrefCodecNum(4) != "-1") msg += aRMap + sessionC->getPrefCodecNum(4) + " " + sessionC->getPrefCodec(4) + "/" + auEnd;
	msg += aRMap + tEvent + "/" + auEnd;
        if (sessionC->getVidPrefCodec(0) == getVidPrefCodec()) {
        	msg += mVideo+port+trsp+ sessionC-> getNZVidPrefCodecNum (0)+ " " + sessionC->getNZVidPrefCodecNum (1)+
		" " + sessionC->getVidPrefCodecNum (2) + end;
        } else if (sessionC->getVidPrefCodec(1)== getVidPrefCodec() ) {
		msg += mVideo+port+trsp+ sessionC-> getNZVidPrefCodecNum (1)+ " " + sessionC->getNZVidPrefCodecNum (0)+
		" " + sessionC->getVidPrefCodecNum (2)+ "/" + end;
	} else         {
        	msg += mVideo+port+trsp+ sessionC-> getNZVidPrefCodecNum (2)+ " " + sessionC->getNZVidPrefCodecNum (0)+
		" " + sessionC->getNZVidPrefCodecNum (1) + end;
        }
	
	if(sessionC->getILBC()) {
		if(getResponsePay() == 240) {
			msg += "a=fmtp:97 mode=30\r\n";
		} else {
			msg += "a=fmtp:97 mode=20\r\n";
		}
	}
	
	if(sessionC->getVidPrefCodecNum(0) != "-1") msg += aRMap + sessionC->getVidPrefCodecNum(0) + " " + sessionC->getVidPrefCodec(0) + "/" + viEnd;
	if(sessionC->getVidPrefCodecNum(1) != "-1") msg += aRMap + sessionC->getVidPrefCodecNum(1) + " " + sessionC->getVidPrefCodec(1) + "/" + viEnd;
	if(sessionC->getVidPrefCodecNum(2) != "-1") msg += aRMap + sessionC->getVidPrefCodecNum(2) + " " + sessionC->getVidPrefCodec(2) + "/" + viEnd;
} else {
	int posN=sessionC->theVectorId(load).find( '\n' ) + 1;
        vecID = sessionC->theVectorId(load);
	vecID = vecID.remove( 0, posN );
	int trans=sessionC->useL4(load);
	if(trans==useudpL4){
			transp=" udp ";
	} else {
			transp=" RTP/AVP ";
	}

	msg += mIs + " " +  port  + transp + vecID + end;

	if(call->isRemoteStart()) {
            msg += "a=fmtp:" + vecID + " rstart" + end;
        }

 }
	if(isHold) {
		    msg += "a=recvonly\r\n";  // We were put on hold, sendonly req.
	} else {
        	msg += "a=sendrecv\r\n";     //  Full duplex call
	}

return msg;
}
int SdpBuild::checkVectorId(int stat, QString incomingId) {
	int b = SipCall::extCall1-1;
	int i = 1;
	for(i = 1 ; i < 5; i++){
	
	    if(incomingId == sessionC->theVectorId(b+i)) return b+i;
	}
	return 0;
}	


int SdpBuild::checkCodec( bool testReInvite, SipCallMember *member, bool forceHold , bool *srtpErr)
{
	bool holdMe=false;
	cntParam=0;
	setRecvonly(false);
	ReInvite=testReInvite;
        int status = (int)SipCall::GarKeinCall;
	int isRtpMap=0;
	int i=0, j=0;
	QString transT;
        bool evaluateV=false, evaluateA=false, evaluateAV=false;
        QString mstr = member->getSessionDescription();
        QString ipaddress;
        QString curline;
        QString portstr;
        QString codecstr;
        QString codecstr1;
        QString codecstr2;
        int receivedAudioMediaNumber = 0, receivedVideoMediaNumber = 0;
        QString dp=":";
	QString sec;
        mstr += '\n';                        // Guarentee termination
        if( mstr.left( mstr.find( '\n' ) ) != "v=0" ) {
	return doReturn (status);
        }
        mstr.remove( 0, mstr.find( '\n' ) + 1 );

        while( mstr.length() > 2 ) {
                curline = mstr.left( mstr.find( '\n' ) );
                mstr.remove( 0, mstr.find( '\n' ) + 1 );
	        if( curline.left( curline.find( '=' ) ) == "c" ) {
            		if( curline.contains("IP4") ) {
		        ipaddress = curline.mid( curline.find( "IP4" ) + 4 ).stripWhiteSpace();
			} else {
		        ipaddress = curline.mid( curline.find( "IP6" ) + 4 ).stripWhiteSpace();
			}

		}
		if( curline.contains("m=audio")) {
			if(ipaddress.contains("0.0.0.0") || ipaddress.contains("::0") || forceHold){
                                status = (int) SipCall::putMeOnHold;
				setExtHost(ipaddress);
				holdMe = true;
			} else {
				if(evaluateV==true) {
					evaluateAV=true;
					status = (int)SipCall::auviCall;
				} else {
					status = (int)SipCall::StandardCall;
					evaluateA=true;
				}
				if ( ( curline.contains("RTP/SAVP") && (sessionC->getSrtpMode() != 1) )|| ( curline.contains("RTP/AVP") && (sessionC->getSrtpMode() != 0) ) ) {
				    *srtpErr = true;
				}
				setAudioPort(curline.section(' ',1,1));
				setExtHost(ipaddress);
				int i = (-1);
				do {
					i++;
					acc[i] = curline.section(' ', i+3, i+3);
					
				} while (acc[i].length() != 0);
				receivedAudioMediaNumber = i;
					
			}
                }
                if( curline.contains("m=video")) {
			if(ipaddress.contains("0.0.0.0")|| ipaddress.contains("::0") || forceHold){
                        	status = (int) SipCall::putMeOnHold;
				setExtHost(ipaddress);
				holdMe = true;
			} else {
				if(evaluateA==true) {
					evaluateAV=true;
					status = (int)SipCall::auviCall;
				} else {
					status = (int)SipCall::videoCall;
					evaluateV=true;
				}
				setExtPort(curline.section(' ',1,1));
				setExtHost(ipaddress);
				int i = (-1);
				do {
					i++;
					vcc[i] = curline.section(' ', i+3, i+3);
					
				} while (vcc[i].length() != 0);
				receivedVideoMediaNumber = i;
                        }
                } 
                if( ( curline.contains("m=application") ) ) {
			if(ipaddress.contains("0.0.0.0") || ipaddress.contains("::0") || forceHold){;
                	     	status = (int) SipCall::putMeOnHold;
				setExtHost(ipaddress);
				holdMe = true;
			} else {
				setExtHost(ipaddress);
                        	setExtPort(curline.section(' ',1,1));
				transT=curline.section(' ',2,2);
				sec=curline.section(' ',3,3);
				status = checkVectorId(status,sec); // find the load type
				call->setRemoteStart(false); //
				if(status==0){
					return  doReturn (status); // error
				}
				if(!testTrans(status,transT))  return  doReturn (0);
				
			}
		}                
		if( curline.contains("a=")) {
			if (curline.contains("a=sendonly") || curline.contains("a=inactive") || curline.contains("a=recvonly") ) {
                    	    status = (int) SipCall::putMeOnHold;
			    if( curline.contains("a=sendonly")) {
			    setRecvonly(true);
			    }
			    if(KStatics::haveIPv6){
				setExtHost( "::0");
			    } else { 
				setExtHost("0.0.0.0");
			    }
    			    return  doReturn (status);
			}
			if (holdMe) {
			    return  doReturn (status); //hold was detected by ipaddress
			}
 
			//
			if (curline.contains("a=fmtp:" + sec) ) {// we have a=vec or a=cmd
				if(curline.contains("rstart")) {
				call->setRemoteStart(true); 
				return  doReturn (status);
				}
			}
			// we look for audio and video
			if( curline.contains("a=rtpmap")) {
				isRtpMap++;
				codecstr=curline.section(' ',1,1);
				codecstr1=curline.section(':',1,1);
				codecstr2=codecstr1.section(' ',0,0);
				// STORE RECEIVED MEDIA ATTRIBUTES
				bool concerningAudio = false;
				// search for audio
				i=0;
				while (  i<receivedAudioMediaNumber) {
					if (codecstr2 == acc[i]) {
						acstr[i] = codecstr.section('/',0,0);
						concerningAudio = true;
						break;
					} 
					i++;
				}
                     		// search for video, if not audio
				if (concerningAudio == false) {
					for (i=0; i<receivedVideoMediaNumber; i++) {
						if (codecstr2 == vcc[i]) {
							vcstr[i] = codecstr.section('/',0,0);
							break;
						}
					}	
				}
    			}
			if ( curline.contains("a=fmtp:97")) {
			// looks strange, but works, as only ILBC uses responsePay
			    if ( curline.contains ("mode=30") ) tempresponsepay = 240;
			    if ( curline.contains ("mode=20") ) tempresponsepay = 160;
			
			}
		}
	} // END OF WHILE-LOOP
	if( (isRtpMap<receivedAudioMediaNumber) && ( evaluateA || evaluateV || evaluateAV) ){
	    for(i=0;i<=receivedAudioMediaNumber;i++) {
		for(j=0;j<=NAUDIO;j++) {
		    if (acc[i] == sessionC->getACodecNum(j) ) {
			if(acstr[i] == "" )acstr[i] = sessionC->getACodecName(j);
			break;
		    }
		}
	    }

	    for(i=0;i<=receivedVideoMediaNumber;i++) {
		for(j=0;j<=NAUDIO;j++) {
		    if (vcc[i] == sessionC->getCodecNum(j) ) {
			    vcstr[i] = sessionC->getCodecName(j);
			    break;
		    }	
		}
	    }
	}

	if( evaluateV || evaluateAV) {


	// *** VIDEO ***
		if (videoMediaNegotiationINVITE(receivedVideoMediaNumber) == false) {
        		status=0;     // no good codec for us; audiocheck is not necessary any longer
			return  doReturn (status);
		}
	}
	if( evaluateA || evaluateAV) {
    	// *** AUDIO ***
        	if (audioMediaNegotiationINVITE(receivedAudioMediaNumber) == false) {
            		status=0;     // no good codec for us;
			return  doReturn (status);
		}
	}
	// We have a good Codec
	return  doReturn (status);
}

// *** A U D I O ***************************************************************************************************************
// Read local supported Payload Type for received AUDIO Encoding Name
QString SdpBuild::readSupportedAudioPayloadType( QString receivedEncodingName ) {

int prio;
        QString localPayloadType = "-1";                                      // init: Payload Type not supported
        int suppMediaNumber = sessionC->getTotalNumberOfPrefAudioCodecs();    // Anzahl aller Audio-Codecs, die wir untersttzen
	prio = 0;
        while  (prio < suppMediaNumber) {
           if (receivedEncodingName.upper() == sessionC->getPrefCodec(prio)) {
              localPayloadType = sessionC->getPrefCodecNum(prio);
              break;
           }
	   prio++;
        }
        return localPayloadType;
}

// Negotiation of AUDIO Codecs
bool SdpBuild::audioMediaNegotiationINVITE(int recAttributeNumber) {
        bool success = false;
        int negPriority = 0;   // start with highest priority of negotiation codecs
        int recAttrCounter = 0;
	while (recAttrCounter < recAttributeNumber)  { // get received Encoding Name
	   QString recEncoding = acstr[recAttrCounter];  
           QString localPayload = readSupportedAudioPayloadType( recEncoding );// get local Payload Type for the received Encoding Name
           if (localPayload != "-1") {// Encoding supported?
              // Highest priority (1st coincidence)
              if (negPriority == 0) {
                 setPrefCodec(recEncoding);
                 setPrefCodecNum(acc[recAttrCounter]); // PAYLOAD TYPE NUMBER OF THE Partner
                 success = true; 
              }
              negPriority++;        // for next entry in Negotiation Parameter file
           } 
	   recAttrCounter++;
        }
	if (getPrefCodecNum() == "97") {
	
	    if ( sessionC->getPayload() != tempresponsepay ) {
		success = false;
	    } else {
	    setResponsePay(tempresponsepay);
	    }
	}
	return success;
}

// *** V I D E O ***************************************************************************************************************
// Read local supported Payload Type for received VIDEO Encoding Name
QString SdpBuild::readSupportedVideoPayloadType( QString receivedEncodingName ) {
        QString localPayloadType = "-1";                                      // init: Payload Type not supported
        int suppMediaNumber = sessionC->getTotalNumberOfPrefVideoCodecs();    // Anzahl aller Video-Codecs, die wir untersttzen
	for (int prio = 0; prio < suppMediaNumber; prio++) {
           if (receivedEncodingName.upper() == sessionC->getVidPrefCodec(prio)) {
              localPayloadType = sessionC->getVidPrefCodecNum(prio);
              break;
           }
        }
       return localPayloadType;
}

// Negotiation of VIDEO Codecs
bool SdpBuild::videoMediaNegotiationINVITE(int recAttributeNumber) {
        bool success = false;
        int negPriority = 0;   // start with highest priority of negotiation codecs
        for (int recAttrCounter = 0; recAttrCounter < recAttributeNumber; recAttrCounter++) {
           QString recEncoding = vcstr[recAttrCounter]; // get received Encoding Name
           QString localPayload = readSupportedVideoPayloadType( recEncoding ); // get local Payload Type for the received Encoding Name
           // Encoding supported?
           if (localPayload != "-1") {
              // Highest priority?
              if (negPriority == 0) {
                 setVidPrefCodec(recEncoding);
                 setVidPrefCodecNum(vcc[recAttrCounter]);// PAYLOAD TYPE NUMBER VON DER ANDEREN SEITE!!!!!
                 success = true;
              }
              negPriority++;        // for next entry in Negotiation Parameter file
           }
        }
        return success;
}

void SdpBuild::setExtHost( const QString &tc ){
	if (tc != testHost) cntParam ++;
	testHost = tc;
}

void SdpBuild::setExtPort( const QString &tp ) {
	if (tp != testPort) cntParam ++;
	testPort = tp;
}

void SdpBuild::setExtHomePort( const QString &thp ) {
	if( thp  == QString::null ) {
		testHomePort = "0";
	} else {
		testHomePort = thp;
	}
}

void SdpBuild::setSExtHomePort( const QString &thp ) {
	if( thp  == QString::null ) {
		testSHomePort = "0";
	} else {
		testSHomePort = thp;
	}
}

void SdpBuild::setAudioPort( const QString &tp ) {
	if (tp != audioPort) cntParam ++;
	audioPort = tp;
}

void SdpBuild::setAudioHomePort( const QString &thp ){
	if( thp  == QString::null ) {
		audioHomePort = "0";
	} else {
		audioHomePort = thp;
	}
}
 
 void SdpBuild::setVidCodec  (const QString &vc0 ){
	if( vc0  == QString::null ) {
		videoCodec = "0";
	} else {
		videoCodec = vc0;
	}
}



codecType SdpBuild::getRtpCodec(void) {
	if (getPrefCodecNum()== "3") return  codecGSM;
	if (getPrefCodecNum()== "8") return  codecALAW;
	if (getPrefCodecNum()== "97") {
		if(sessionC->getPayload() == 160) return  codecILBC_20; else return  codecILBC_30;
	}
	if (getPrefCodecNum()== "98") return  codecSpeex;
	return  codecULAW;
}

codecType SdpBuild::getVideoRtpCodec(void) {
return  codecULAW;
}

int  SdpBuild::getRtpCodecNum(void) {
if (getPrefCodecNum()== "3") return  3;
if (getPrefCodecNum()== "8") return  8;
if (getPrefCodecNum()== "97") return  97;
if (getPrefCodecNum()== "98") return  98;
return  0;
}

bool SdpBuild::testTrans(int status,QString trans) {
int tr = sessionC->useL4(status);
	if ( (tr == useudpL4) && (trans.lower() == "udp") ) return true;
	if ( (tr == usertpL4) && (trans.lower() == "rtp/avp") ) return true;
	return false;
}

void SdpBuild::setVidPrefCodec (const QString &vc) {
if (vc != videoPrefCodec) cntParam ++;
videoPrefCodec = vc; 
}

void SdpBuild::setVidPrefCodecNum (const QString &vc) {
if (vc != videoPrefCodecNum) cntParam ++;
videoPrefCodecNum = vc; 
}

void SdpBuild::setPrefCodec (const QString &pc) {
if (pc != prefCodec) cntParam ++;
prefCodec = pc; 
}


void SdpBuild::setPrefCodecNum (const QString &pc) {
if (pc != prefCodecNum) cntParam ++;
prefCodecNum = pc; 
}

int SdpBuild::doReturn (int stat) {
	realLoad=stat;
	if( ReInvite) {
		if( (stat == 0) || (stat == (int) SipCall::putMeOnHold) ) {
		     return stat; 
		} else {
		    if (cntParam >0 ) return stat;
		    return (int)SipCall::noChange;
		}
	} else return stat;
}

void SdpBuild::setResponsePay(int pay) {
if ( pay != responsePay) cntParam ++;
responsePay = pay;
}

/*QString SdpBuild::prepOptString(void) {
	msg = "Accept: application/sdp\r\n";
	msg += "Accept-Language: en\r\n";
	msg += "Supported: eventlist\r\n";
return msg;
}
*/


QString SdpBuild::prepOptString(void) {
        msg = "v=0\r\n"; // Version
        msg += "o=username 0 0 IN IP4 0.0.0.0\r\n";
        msg += "s=The Funky Flow\r\n"; // Session Name
        msg += "c=IN IP4 0.0.0.0\r\n";
        msg += "t=0 0\r\n"; // Time active
	msg += "audio 0 RTP/AVP 0 8 3 97 98 101\r\n";
	msg += "a=rtpmap:0 PCMU/8000\r\n";
	msg += "a=rtpmap:8 PCMA/8000\r\n";
	msg += "a=rtpmap:3 GSM/8000\r\n";
	msg += "a=rtpmap:97 ILBC/8000\r\n";
	msg += "a=rtpmap:98 SPEEX/8000\r\n";
	msg += "a=rtpmap:101 telephone-event/8000\r\n";
//	msg += "a=fmtp:97 mode=20\r\n";
//	msg += "m=video 8000 RTP/AVP 31  34  103\r\n";
//	msg += "a=rtpmap:31 H261/90000\r\n";
//	msg += "a=rtpmap:34 H263/90000\r\n";
//	msg += "a=rtpmap:103 H263-1998/90000\r\n";
return msg;
}
