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
#ifdef SRTPWRAPPER_HX
#include <qmutex.h>
#include <string>
#include "../kaudio/rtpdataheader.h"
#include "../config.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * Because the libsrtp is written in C, it was necessary to define
 * some C++ keywords which are used as variable names in the library
 */
#define new k_new
#define protected k_protected
#ifdef SRTP
#include "/usr/local/include/srtp/srtp.h"
#endif
#undef protected
#undef new
#ifdef __cplusplus
};
#endif


/**
 * SRTPWrapper is designed to link a SIP/RTP-client with the libsrtp library,
 * to allow a secure VoIP connection using SRTP.
 * This class is designed as singleton.
 */
class SRTPWrapper{
	public:    
        /**
         * Returns a pointer of the actual SRTPWrapper object (singleton).
         */
        static SRTPWrapper* getInstance();
        /**
         * Encrypts a RTP packet to a SRTP packet and adds a 10Bytes MAC.
         * (Calls the srtp_protect method from the libsrtp library)
         */
        void protect(unsigned char* rtp_h, int* length);

        /**
         * Decrypts a SRTP packet to a SRTP packet and removes the 10Bytes MAC.
         * (Calls the srtp_unprotect method from the libsrtp library)
         */
        void unprotect(unsigned char* rtp_h, int* length);
        
#ifdef SRTP
        /**
         *The key management sets SRTP policy using this method 
         */ 
        void setPolicy(srtp_policy_t* pol);
#endif        
        /**
         * Decreases instanceCount and deletes the singleton instance if there
         * is only one reference left.
         */
        void dispose(void);
        
        /**
         * Deletes the instance of SRTPWrapper if there are 0 references.
         */
        void destroy(void);
		
	protected:
		/**
		 * Default constructor, protected because of singleton.
		 */
		SRTPWrapper();
		
		/**
		 * Destructor, protected because of singleton.
		 * The destructor can be triggered using the dispose method.
		 */
		~SRTPWrapper();
		
	private:            

        /**
         * Prints out the actual SRTP policy information 
         */
        void printPolicyInfo(void);
        /**
         * Pointer to the singleton instance.
         */         
        static SRTPWrapper* instance;
        

#ifdef SRTP
        /**
         * Copy constructor, private because of singleton.
         */
        SRTPWrapper(const SRTPWrapper&);
        
        /**
         * Creates a new SRTP session. Requires local SRTP policy to be set
         * by key management. (extension)
         */
        err_status_t newSession();
        
        /**
         * Creates a new SRTP session (calls the srtp_create method from
         * the libsrtp library).
         */
        err_status_t newSession(ssrc_t ssrc);
        
        /**
         * Reads the key
         */
        octet_t* readKey();
        
        /**
         * Returns the name of the error value from libsrtp.
         */
        std::string getErrorname(err_status_t err);
        
        /**
         * If an function of libSRTP returns an error value, it's 
         * reported to the user by this function. 
         */
        void libError(err_status_t err);
        
        /**
         * Counter variable for the references.
         */
        static int countReferences;
       
       /**
        * Shows if an SRTP session is already active.
        */
        bool sessionActive;
        
        /**
        * Shows if an outbound stream is already active.
        */
        bool outboundStreamActive;
        
        /**
        * Shows if an inbound stream is already active.
        */
        bool inboundStreamActive;
        
        /**
         * The handler for the active SRTP session
         */
        srtp_t session;
        
        /**
         * Random integer because of kphone uses a fix ssrc value  for all
         * streams and libsrtp does only work with ssrc which are different for
         * different streams
         */
        int localSSRC;
        
        /**
         * Errornames from the libsrtp errors.
         */
        static const std::string err_array[16];
        
        /**
         * Shows if the srtp_init() method have already been executed.
         */
        static bool libInit;
        
        /**
         * SRTP policy informations set by the Key Management
         */
         srtp_policy_t* policy;
         
        
        #ifdef QT_THREAD_SUPPORT
        /**
         * Mutex to lock the session set-up
         */
        QMutex mutex;
        #endif
#endif
};

#endif // SRTPWRAPPER_HX
