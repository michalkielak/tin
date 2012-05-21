
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <features.h>
#include <net/if_arp.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <qmessagebox.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <siputil.h>
#include <messagesocket.h>
#include "../kphone/kstatics.h"
#include <iostream>
using namespace std;

typedef struct {
	char name[21];  // iface name
	char addr[71];  // address as ASCII
	int  af;        // AF_INET  or  AF_INET6  or  0 (invalid)
} iface_cfg;


static char *dissipate_our_fqdn  = NULL;
//static char *dissipate_our_fqdn6 = NULL;
static iface_cfg *dissipate_last_ifaces  = NULL;
static iface_cfg *dissipate_last_ifaces6 = NULL;

/* max number of network interfaces*/
#define MAX_IF 16

/* Path to the route entry in proc filesystem */
#define PROCROUTE "/proc/net/route"

/* file containing the hostname of the machine */
/* This is the name for slackware and redhat */

#define HOSTFILE "/etc/HOSTNAME"

/* and this is the name for debian */
/* #define HOSTFILE "/etc/HOSTNAME" */

#ifndef SIOCGIFCOUNT
#define SIOCGIFCOUNT 0x8935
#endif

char iface[16];

int SipUtil::found=0;
bool SipUtil::checkFilename( const char *filename )
{
	int fd;
	fd = open( filename, O_WRONLY );
	if ( fd == -1 ) {
		return false;
	}
	// else
	close( fd );
	return true;
}


/* This searches the proc routing entry for the interface the default gateway
* is on, and returns the name of that interface.
*/
char *getdefaultdev()
{
	FILE *fp = fopen( PROCROUTE, "r");
	char buff[4096], gate_addr[128], net_addr[128];
	char mask_addr[128];
	int irtt, window, mss, hh, arp, num, metric, refcnt, use;
	unsigned int iflags;
	char i;
	if( !fp ) {
		perror("fopen");
		return NULL;
	}
	i=0;

// cruise through the list, and find the gateway interface
	while( fgets(buff, 1023, fp) ) {
		num = sscanf(buff, "%s %s %s %X %d %d %d %s %d %d %d %d %d\n",
			iface, net_addr, gate_addr, &iflags, &refcnt, &use, &metric,
			mask_addr, &mss, &window, &irtt, &hh, &arp);
		i++;
		if( i == 1) continue;
		if( iflags & RTF_GATEWAY )
			return iface;
	}
	fclose(fp);
/* didn't find a default gateway */
	return NULL;
}


unsigned int nybbleValue(char nybble) {
	if (nybble>='0'&&nybble<='9') return nybble-'0';
	if (nybble>='A'&&nybble<='F') return nybble-'A'+10;
	if (nybble>='a'&&nybble<='f') return nybble-'a'+10;
	return 0;
}


// returns pointer to multiple iface_cfg structures, af member of last structure is 0.
// returns NULL on failure.
// caller must free() returned pointer.
iface_cfg * getIpAddressList( int af, int *count ) 
{
	int sifr  =  sizeof(struct ifreq);
	int sock, err, if_count;
	struct ifconf netconf;
	char buffer[sifr*MAX_IF];
	iface_cfg *ret=NULL;
	struct ifreq *ifr;
	int i=0, j=0, k=0, n=0;
	char *devclasses[]={ (char *) "wlan", (char *) "eth", (char *) "ppp", NULL };
	char *devcl;
	int devcli;


	FILE *fd=NULL;
	char ip6addr[33];
	unsigned char dummy[100];
	int ign;                   // flag


	if (af==AF_INET) {
		// retrieve the IPv4 interfaces and addresses
		netconf.ifc_len = sifr * MAX_IF;
		netconf.ifc_buf = buffer;
		sock=socket( PF_INET, SOCK_DGRAM, 0 );
		err=ioctl( sock, SIOCGIFCONF, &netconf );
		if ( err < 0 ) { 
			cout  <<  "!!!!!Error in ioctl: " << errno << endl; 
			return NULL; 
		}
		if_count = netconf.ifc_len / sifr;//32;
		ret = ( iface_cfg * ) malloc( sizeof( iface_cfg ) * ( if_count + 1 ) );
		if (!ret) return NULL;
		for (devcli=0;;devcli++) {
			devcl=devclasses[devcli];
			for (i=j=0; i<if_count; i++) {
				ifr=netconf.ifc_req+i;
				if ( devcl && strncmp(devcl, ifr->ifr_name, strlen(devcl)) ) continue;
				strncpy( ret[j].name, ifr->ifr_name, 20 );
				if (!strcmp("lo",ret[j].name)) continue;
				inet_ntop(AF_INET, &((struct sockaddr_in *)(&ifr->ifr_addr))->sin_addr, ret[j].addr, 20);
				if (ioctl (sock, SIOCGIFFLAGS, ifr)) { perror ("ioctl:"); continue; }
				if ( (ifr->ifr_flags & IFF_UP ) == 0 ) continue;        // ignore down interface
				ret[j].af=af;
				j++;
			}
			if (!devcl) break;
		}
		close( sock );
		ret[j].af=0;
		*count=j;
		return ret;
	} else if (af==AF_INET6) {
		*count=0;
		fd=fopen("/proc/net/if_inet6","r");
		if (!fd) {
			perror("fopen(\"/proc/net/if_inet6\",\"r\")");
			return NULL;
		}
		j=k=0;
		while (1) {
			ret = ( iface_cfg * ) realloc( ret, sizeof( iface_cfg ) * ( k + 1 ) );
			if (!ret) return NULL;
			n=fscanf(fd,"%s %s %s %s %s %s",ip6addr,dummy,dummy,dummy,dummy,ret[k].name);
			if (n!=6) {
				fclose(fd);
				*count=k;
				ret[k].af=0;
				return ret;	
			}
			if (!strcmp("lo",ret[k].name)) continue;
			for (i=0; i<32; i+=2) {
				dummy[i>>1]=(nybbleValue(ip6addr[i])<<4)+nybbleValue(ip6addr[i+1]);
			}
			// for sanity ignore link local and multicast addresses
			ign=0;
			if (dummy[0]==0xff) ign=1; // multicast

//			else if ( dummy[0]==0xfe && (dummy[1]&0xc0)==0x80 ) ign=1; // link local
			if (!ign) {
				inet_ntop(AF_INET6, dummy, ret[k].addr, 70);
				ret[k].af=af;
				k++;
			}
		}
	}

	return NULL;

}


char *IPdialog( iface_cfg *ifaces, int if_count )
{
	int i=0;
	QString Cnt = QString::number(if_count);
	for (;;) {
		if(if_count==1) 	return strdup( ifaces[i].addr ); 
		QMessageBox mb( "Interface Selection",
		"We have "+ Cnt  + " Interfaces \n Do you want to use " + QString(ifaces[i].name) + ((ifaces[i].af==AF_INET)?"  (IP:\"":"  (IP6:\"") + QString(ifaces[i].addr) + "\")",			QMessageBox::Information,
			QMessageBox::Yes | QMessageBox::Default,
			(if_count>1)?QMessageBox::No:(QMessageBox::Cancel | QMessageBox::Escape),
			(if_count>1)?(QMessageBox::Cancel | QMessageBox::Escape):0 );
		mb.setButtonText( QMessageBox::Yes, "OK" );
		mb.setButtonText( QMessageBox::Yes, "OK" );
		if (if_count>1) mb.setButtonText( QMessageBox::No,  (i==if_count-1)?"Start over":"Next" );
		switch( mb.exec() ) {
			case QMessageBox::No:
				i++; 
				if (i==if_count) i=0;
				break;
			case QMessageBox::Yes:
				return strdup( ifaces[i].addr ); 
			case QMessageBox::Cancel:
				return NULL;
		}
	}

}


void findFqdn( int af )
{
	int if_count,i;
	iface_cfg *ifaces;
	char **fqdn=&dissipate_our_fqdn;
	iface_cfg **last_ifaces=(af==AF_INET)?&dissipate_last_ifaces:&dissipate_last_ifaces6;

	ifaces=getIpAddressList (af, &if_count);

	for (i=0; i<if_count; i++) {
		QString afs;
		if(af==AF_INET) afs = "v4"; else afs="v6";
		if (KStatics::debugLevel >= 2) cout << "!!!!!IP" << afs << " Device: " <<   ifaces[i].name << " " <<ifaces[i].addr << endl;
	}

	if (if_count>0) *fqdn = IPdialog ( ifaces, if_count );
	else *fqdn=NULL;

	if (*last_ifaces) { free( *last_ifaces ); }
	*last_ifaces = ifaces;
	return;
}


char *SipUtil::getLocalFqdn( void )
{
bool ipv6=KStatics::haveIPv6;
if(ipv6) {
	if (found) goto ret6;
	if ( dissipate_our_fqdn == NULL ) {
		findFqdn( AF_INET6 );
	}
ret6:
	return dissipate_our_fqdn;

} else {
	if (found) goto ret4;
	if ( dissipate_our_fqdn == NULL ) {
		findFqdn( AF_INET );
	}
ret4:
	return dissipate_our_fqdn;
}
}

char *SipUtil::getLocalFqdn6( void )
{
	static int found=0;

	if (found) goto ret;
	if ( dissipate_our_fqdn == NULL ) {
		findFqdn( AF_INET6 );
	}
	found=1;
ret:
	return dissipate_our_fqdn;
}


void dumpAddressList (iface_cfg *l, char *name) {
	int i;

	cout << "Dumping Address list " << name << l << endl;
	for (i=0; l[i].af; i++) {
		cout << i << l[i].name << l[i].addr << endl;
	}
}

// please don't remove

// return 1 on change, 0 if no change was detected
// if a change was detected, the global dissipate variables have already been updated !!!
//
/*int SipUtil::checkForAddressChange (int af) 
{
	iface_cfg **lastp,*current, *l;
	int count,i,j,r=0,cx=0;
	char **lastaddrp;

	if (af==AF_INET) {
		lastp=&dissipate_last_ifaces;
		lastaddrp=&dissipate_our_fqdn;
	} else {
		lastp=&dissipate_last_ifaces6;
		lastaddrp=&dissipate_our_fqdn6;
	}
//	dumpAddressList(*lastp,(char *)"lastp");
//	printf ("lastaddr=%s\n",*lastaddrp);
	if (!(current=getIpAddressList(af, &count))) goto ret;
//	dumpAddressList(current,(char *)"current");

	// 1. check if the currently used address is in the current list
	if (!count) { free(current); goto ret; } // No addresses of the specified AF present
	if (*lastaddrp)	
		for (i=0; i<count; i++) {
			if (!strcmp(*lastaddrp,current[i].addr)) goto fret; // found it -> no change
		}

	// 2. So our address has vanished or we did not have one -> find the address by which it was replaced.
	if (*lastp) {
		for (i=0,r=1; i<count; i++) {
			for (cx=j=0;;j++) {
				l=(*lastp)+j;
				if (!l->af) break; // end of last list
				if (!strcmp(l->addr,current[i].addr)) { cx=1; break; }
			}
			if (!cx) break;  // found an entry that was not present before
		}
		if (i==count) i=0;     // address has just vanished and was not replaced
	} else i=0;
	if (*lastaddrp) free(*lastaddrp);
	*lastaddrp=strdup(current[i].addr);

fret:
	if (*lastp) free(*lastp);
	*lastp=current;
ret:
	return r;


}
*/

// please don't remove

// return 1 on change, 0 if no change was detected
// if a change was detected, the global dissipate variables have already been updated !!!
//

int SipUtil::checkForAddressChange2 (int af1, const char *registrarIP) 
{
	iface_cfg *current;
	int count,j,r=0;
	int i = 0;
	char **lastaddrp;
//	int regv;
	char pingcmd[1024];
	char *ra;
	int af;

	lastaddrp=&dissipate_our_fqdn;

	switch (MessageSocket::getSocketIpVersion()) {
	    case 4: af = AF_INET;
		    break;
	    case 6: af = AF_INET6;
		    break;
	    default: af = AF_INET;
		    break;
	}
	cout << " a=" << af << MessageSocket::getSocketIpVersion() << AF_INET << AF_INET6 << "lastaddr " << *lastaddrp << " regIP" << registrarIP << endl;
	if (!(current=getIpAddressList(af, &count))) goto ret;    // No address list can be obtained.
	
	// 1. check if the currently used address is in the current list
	if (!count) { goto inv; } // No addresses of the specified AF present
	if (*lastaddrp)	
		for (i=0; i<count; i++) {
		cout  << "compar " << *lastaddrp << current[i].addr << endl;
			if (!strcmp(*lastaddrp,current[i].addr)) { goto fret; } // found it -> no change
		}
	if (!registrarIP) { i=0; goto lret; }
	cout << "SipUtil::checkForAddressChange2(): pinging over " << count << " interfaces.\n";
	if(af==AF_INET)	{
		for (i=0; i<count; i++) {
			sprintf (pingcmd, "ping -c 1 -w 1 -q -I %s %s", current[i].addr, registrarIP);
			cout << pingcmd << endl;
			j=system (pingcmd);
			if (j<0) { 
				cout << "ping error " << pingcmd << endl; 
				perror(""); 
				continue; 
			}	
			if (!WEXITSTATUS(j)) goto lret; else goto inv;  // success or not
		}
	} else {
	    i--;if (i<0) goto inv;  
	}
lret:
	cout << "SipUtil::checkForAddressChange2(): lret " << i << current[i].addr <<endl;
	ra=current[i].addr;

	r=1;
	if (*lastaddrp) free(*lastaddrp);
	*lastaddrp=strdup(ra);
	goto fret1;
fret:
	cout << "SipUtil::checkForAddressChange2(): fret " << *lastaddrp << endl;
fret1:
	free (current);
	goto ret1;
ret:
	cout << "SipUtil::checkForAddressChange2(): finished " << *lastaddrp << endl;
ret1:
	return r;
inv:
//	ra=(char *)"0.0.0.0";
//	goto cret;
	goto fret;  // currently "invalid" is mapped to "no change"

}

