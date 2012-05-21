/*
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
#include <iostream>
#include <qsettings.h>
#include <cstdlib>
#define SRTPWRAPPER_HX
#include "SRTPWrapper.h"
#include "../config.h"
#include "../kphone/kstatics.h"
using namespace std;

SRTPWrapper* SRTPWrapper::instance = 0;
#ifdef SRTP	
int SRTPWrapper::countReferences = 0;
const string SRTPWrapper::err_array[] = {
	"err_status_ok", 
	"err_status_fail",
	"err_status_bad_param",
	"err_status_alloc_fail",
	"err_status_dealloc_fail",
	"err_status_init_fail",
	"err_status_terminus",
	"err_status_auth_fail",
	"err_status_cipher_fail",
	"err_status_replay_fail",
	"err_status_replay_old",
	"err_status_algo_fail",
	"err_status_no_such_op",
	"err_status_no_ctx",
	"err_status_cant_check",
	"err_status_key_expired"
};
bool SRTPWrapper::libInit = false;
#endif

SRTPWrapper::SRTPWrapper(){
#ifdef SRTP	
	outboundStreamActive = false;
	inboundStreamActive = false;
	sessionActive = false;
	if(!SRTPWrapper::libInit){
		if(err_status_t error = srtp_init()){
			libError(error);
			exit(1);
		}
		SRTPWrapper::libInit = true;
	}
#endif
}

SRTPWrapper::~SRTPWrapper(){
#ifdef SRTP	
		if(sessionActive)
			delete session;
		SRTPWrapper::instance = 0;
#endif
}


void SRTPWrapper::dispose(void){
#ifdef SRTP	
	SRTPWrapper::countReferences--;
#endif
}

void SRTPWrapper::destroy(void){
#ifdef SRTP	
	if(SRTPWrapper::countReferences == 0)
		delete this;	
#endif
}


void SRTPWrapper::protect(unsigned char* rtp_h, int* length){
#ifdef SRTP	
	rtp_hdr_t* hdr = (rtp_hdr_t *) rtp_h;
	QSettings settings;
	if(!sessionActive){
		if(settings.readEntry(KStatics::dBase + "SRTP/Mode", "") == "PSK"){
			localSSRC = rand();
			ssrc_t ssrc;
			ssrc.value = localSSRC;
			ssrc.type = ssrc_any_outbound;
			if (err_status_t err = this->newSession(ssrc)){
			libError(err);
				cout << "SRTP session creation failed (protect)." << endl;
			}else{
				cout << "SRTP session created! (PSK)" << endl;
				sessionActive = true;
			}
		} else if(settings.readEntry(KStatics::dBase + "SRTP/Mode", "") == "PKE"){
				if(err_status_t err = this->newSession()){
					libError(err);
					cout << "SRTP session creation failed (protect) (PKE)." << endl;
				}else{
					cout << "SRTP session created!" << endl;
					sessionActive = true;
				}
		} else {
				cout << "Key exchange Mode not supported: " << 	settings.readEntry(KStatics::dBase + "SRTP/Mode", "") << endl;
				return;
		}
	}
	
	
	if(!outboundStreamActive){
		session->stream_template->direction = dir_srtp_sender;
		outboundStreamActive = true;
	}
	hdr->ssrc = localSSRC; //KPhone uses the same SSRC for all Streams, this produces replay errors in SRTP
	
	if(err_status_t err = srtp_protect(session, rtp_h, length)){
		cout << "srtp_protect: ";
		libError(err);
	}
#endif
}

void SRTPWrapper::unprotect(unsigned char* rtp_h, int* length){
#ifdef SRTP	
	if(!sessionActive){
			cout << "Session not active for unprotect. Waiting for outgoing traffic.." << endl;
			//Mute packet to avoid amplitude
			for (int i=0; i< *length ; i++){
				rtp_h[i] = 0xff;	
			}
	}else{
		if(!inboundStreamActive){
			session->stream_template->direction = dir_srtp_receiver;
			inboundStreamActive = true;
		}
		err_status_t err = srtp_unprotect(session, rtp_h, length);	
		if(err){
			cout << "srtp_unprotect: ";
			libError(err);
			//Mute packet to avoid amplitude
			for (int i=0; i< *length ; i++){
				rtp_h[i] = 0xff;	
			}
		}
	}
#endif
}

#ifdef SRTP	
void SRTPWrapper::setPolicy(srtp_policy_t* pol){
	policy = pol;
	localSSRC = policy->ssrc.value; 
}


void SRTPWrapper::libError(err_status_t err){
	cout << "WARNING: libSRTP reported an Error: Code "<< err << " (" << 
		getErrorname(err) << ")!" << endl;
}
#endif

void SRTPWrapper::printPolicyInfo(){
#ifdef SRTP	
	QSettings settings;
	cout << "SRTP Policy (RTP): " << endl;
	cout << "Key Exchange Mode: " << settings.readEntry(KStatics::dBase + "SRTP/Mode", "");
	cout << ", SSRC: " << policy->ssrc.value;
	cout << ", SSRC type : " << policy->ssrc.type << endl;
	cout << "cipherKeylengh: " << policy->rtp.cipher_key_len;
	cout << ", cypherType: " << policy->rtp.cipher_type << endl;
	cout << "AuthKeyLen: " << policy->rtp.auth_key_len;
	cout  << ", AuthType: " << policy->rtp.auth_type;
	cout  << ", AuthTagLengh: " << policy->rtp.auth_tag_len << endl;
	cout << "SRTP Master Key: " ;
	cout.setf(ios::hex, ios::basefield);
	for (int i=0; i < policy->rtp.cipher_key_len ; i++){
		cout <<	(short) policy->key[i];
	}
	cout << endl;
	cout.setf(ios::dec, ios::basefield);
#endif
}


//-------------

SRTPWrapper* SRTPWrapper::getInstance(){
#ifdef SRTP	
	if(SRTPWrapper::instance == 0) {
		SRTPWrapper::instance = new SRTPWrapper();
	}
	countReferences++;
#endif
	return SRTPWrapper::instance;
}
#ifdef SRTP	

//Case PKE
err_status_t SRTPWrapper::newSession(){
	session = new srtp_ctx_t();
	err_status_t err = err_status_ok;
	
	if(policy){
		printPolicyInfo();
		err = srtp_create(&session, policy);
	}else{
		err = err_status_no_ctx;
		cout << "ERROR: Cannot start SRTP session, policy was not set by key management." << endl;
	}
	return err;
}

//Case PSK
err_status_t SRTPWrapper::newSession(ssrc_t ssrc){
	cout << "SSRC for new Session: " << ssrc.value << endl;
	session = new srtp_ctx_t();
	policy = new srtp_policy_t;
	crypto_policy_set_rtp_default(&policy->rtp);
	crypto_policy_set_rtcp_default(&policy->rtcp);
	policy->ssrc.value = ssrc.value;
	policy->ssrc.type = ssrc.type;
	policy->key = this->readKey();
	policy->next = NULL;
	printPolicyInfo();
	err_status_t err = srtp_create(&session, policy);
	return err;
}


octet_t* SRTPWrapper::readKey(){
	QSettings settings;
	QString qkey = settings.readEntry(KStatics::dBase + "SRTP/KeyValue", "");
	string test = qkey;
	if(test.length() < 30){
		int i = 30 - test.length();
		test.append(i, '0');
		cout << "WARNING: The chosen key is to short (<30) and will be padded!" << endl;
	}
	string* strKey = new string(test);
	unsigned char* tmp = (unsigned char*) strKey->c_str();
	return tmp;
}

string SRTPWrapper::getErrorname(err_status_t err){
	return SRTPWrapper::err_array[err];
}

#endif
