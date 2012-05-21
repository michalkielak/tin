
#include "sipclient.h"
#include "sipuser.h"

SipUser::SipUser( SipClient *parent, SessionControl *sc, QString fullname, QString username, QString athostname )
{
	client = parent;
	sessionC = sc;
	myuri.setFullname( fullname );
	myuri.setUsername( username );
	myuri.setHostname( athostname );
	client->setUser( this );
}

SipUser::SipUser( SipClient *parent, SessionControl *sc, const SipUri &inituri )
{
	client = parent;
	sessionC = sc;
	myuri = inituri;
	client->setUser( this );
}

SipUser::~SipUser( void )
{
}

void SipUser::addServer( SipRegister *server )
{
	servers.append( server );
}

void SipUser::removeServer( SipRegister *server )
{
	servers.remove( server );
}

void SipUser::setUri( const SipUri &newuri )
{
	myuri = newuri;
}
