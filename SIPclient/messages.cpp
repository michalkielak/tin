#include <iostream>
#include <cstring>
#include <sstream>
using namespace std;

class Messages{
	public:
	string registerMsg;
	string registerAuthMsg;
	string msg3, msg2;
	string invite, invite2, invite3;
	string ack, bye;
	string ok;
	string myIp, /*otherIp,*/ serverIp, myLogin, otherLogin;
	string contentSize;

	Messages(string myLogin, string myIp, string otherLogin, /*string otherIp,*/ string serverIp)
	{
		this->myIp = myIp;
	//	this->otherIp = otherIp;
		this->serverIp = serverIp;
		this->myLogin = myLogin;
		this->otherLogin = otherLogin;
		//wyliczamy contentSize
		int cs = 376;
		cs+= 2*myIp.size() + myLogin.size();
		ostringstream oss;
		oss << cs;
		contentSize = oss.str();
	}

	string getRegisterMsg()
	{
		registerMsg = "REGISTER sip:"+ serverIp +" SIP/2.0\r\n";
		registerMsg+="Via: SIP/2.0/UDP "+ myIp +":8060;rport;branch=z9hG4bK1606792150\r\n";
		registerMsg+="From: <sip:"+ myLogin +"@"+ serverIp +">;tag=697264525\r\n";
		registerMsg+="To: <sip:"+ myLogin +"@"+ serverIp +">\r\n";
		registerMsg+="Call-ID: 2082372650\r\n";
		registerMsg+="CSeq: 1 REGISTER\r\n";
		registerMsg+="Contact: <sip:"+ myLogin +"@"+ myIp +":8060;line=171085286da3c77>\r\n";
		registerMsg+="Max-Forwards: 70\r\n";
		registerMsg+="User-Agent: Linphone/3.3.2 (eXosip2/3.3.0)\r\n";
		registerMsg+="Expires: 3600\r\n";
		registerMsg+="Content-Length: 0\r\n";
		
		return registerMsg;
		
	}

	string getRegisterAuthMsg(string nonce, string response)
	{
		
		registerAuthMsg = "REGISTER sip:"+ serverIp +" SIP/2.0\r\n";
		registerAuthMsg+="Via: SIP/2.0/UDP "+ myIp +":8060;rport;branch=z9hG4bK1606792150\r\n";
		registerAuthMsg+="From: <sip:"+ myLogin +"@"+ serverIp +">;tag=697264525\r\n";
		registerAuthMsg+="To: <sip:"+ myLogin +"@"+ serverIp +">\r\n";
		registerAuthMsg+="Call-ID: 2082372650\r\n";
		registerAuthMsg+="CSeq: 2 REGISTER\r\n";
		registerAuthMsg+="Contact: <sip:"+ myLogin +"@"+ myIp +":8060;line=0997072b3c9b658>\r\n";
		registerAuthMsg+="Authorization: Digest username=\"stud5\", realm=\"tele.pw.edu.pl\", nonce=\"";
		registerAuthMsg+=nonce;
		registerAuthMsg+="\", uri=\"sip:194.29.169.4\", response=\"" +response+ "\", algorithm=MD5\r\n";
		registerAuthMsg+="Max-Forwards: 70\r\n";
		registerAuthMsg+="User-Agent: Linphone/3.3.2 (eXosip2/3.3.0)\r\n";
		registerAuthMsg+="Expires: 3600\r\n";
		registerAuthMsg+="Content-Length: 0\r\n";

		return registerAuthMsg;
	}

	string getInviteMsg()
	{
		invite ="INVITE sip:"+otherLogin+"@"+serverIp+":8060 SIP/2.0\r\n";
		invite+="Via: SIP/2.0/UDP "+myIp+":8060;rport;branch=z9hG4bK1858780968\r\n";//!
		invite+="From: <sip:"+myLogin+"@"+serverIp+":8060>;tag=1945341956\r\n";
		invite+="To: <sip:"+otherLogin+"@"+serverIp+":8060>\r\n";
		invite+="Call-ID: 1411118635\r\n";
		invite+="CSeq: 20 INVITE\r\n";
		invite+="Contact: <sip:"+myLogin+"@"+myIp+":8060>\r\n";//!
		invite+="Content-Type: application/sdp\r\n";
		invite+="Allow: INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, NOTIFY, MESSAGE, SUBSCRIBE, INFO\r\n";
		invite+="Max-Forwards: 70\r\n";
		invite+="User-Agent: Linphone/3.3.2 (eXosip2/3.3.0)\r\n";
		invite+="Subject: Phone call\r\n";
		invite+="Content-Length:   "+ contentSize +"\r\n";//from 405+4
		invite+="\r\n";
		invite+="v=0\r\n";
		invite+="o="+myLogin+" 123456 654321 IN IP4 "+myIp+"\r\n";//!
		invite+="s=A conversation\r\n";
		invite+="c=IN IP4 "+myIp+"\r\n";//!
		invite+="t=0 0\r\n";
		invite+="m=audio 7078 RTP/AVP 112 111 110 3 0 8 101\r\n";
		invite+="a=rtpmap:112 speex/32000/1\r\n";
		invite+="a=fmtp:112 vbr=on\r\n";
		invite+="a=rtpmap:111 speex/16000/1\r\n";
		invite+="a=fmtp:111 vbr=on\r\n";
		invite+="a=rtpmap:110 speex/8000/1\r\n";
		invite+="a=fmtp:110 vbr=on\r\n";
		invite+="a=rtpmap:3 GSM/8000/1\r\n";
		invite+="a=rtpmap:0 PCMU/8000/1\r\n";
		invite+="a=rtpmap:8 PCMA/8000/1\r\n";
		invite+="a=rtpmap:101 telephone-event/8000/1\r\n";
		invite+="a=fmtp:101 0-11\r\n";

		return invite;
	}

	string getAckMsg(string toTag)
	{	
		ack="ACK sip:"+otherLogin+"@"+serverIp+":8060 SIP/2.0\r\n";
		ack+="Via: SIP/2.0/UDP "+myIp+":8060;rport;branch=z9hG4bK1763228933\r\n";
		ack+="Route: <sip:"+serverIp+":8060;lr>\r\n";
		ack+="From: <sip:"+myLogin+"@"+serverIp+":8060>;tag=1945341956\r\n";
		ack+="To: <sip:"+otherLogin+"@"+serverIp+":8060>;tag="+toTag+"\r\n";
		ack+="Call-ID: 1411118635\r\n";
		ack+="CSeq: 20 ACK\r\n";
		ack+="Contact: <sip:"+myLogin+"@"+myIp+":8060>\r\n";
		ack+="Max-Forwards: 70\r\n";
		ack+="User-Agent: Linphone/3.3.2 (eXosip2/3.3.0)\r\n";
		ack+="Content-Length: 0\r\n";
		
		return ack;
	}

	string getByeMsg(string toTag)
	{
		bye="BYE sip:"+otherLogin+"@"+serverIp+":8060 SIP/2.0\r\n";
		bye+="Via: SIP/2.0/UDP "+myIp+":8060;rport;branch=z9hG4bK1394956433\r\n";
		bye+="Route: <sip:"+serverIp+":8060;lr>\r\n";
		bye+="From: <sip:"+myLogin+"@"+myIp+">;tag=1945341956\r\n";
		bye+="To: <sip:"+otherLogin+"@"+serverIp+":8060>;tag="+toTag+"\r\n";
		bye+="Call-ID: 1411118635\r\n";
		bye+="CSeq: 21 BYE\r\n";
		bye+="Contact: <sip:"+myLogin+"@"+myIp+":8060>\r\n";
		bye+="Max-Forwards: 70\r\n";
		bye+="User-Agent: Linphone/3.3.2 (eXosip2/3.3.0)\r\n";
		bye+="Content-Length: 0\r\n";
		return bye;
	}

	string getOkMsg(string via, string from, string to, string call_id, string c_seq)
	{
		ok="SIP/2.0 200 OK\r\n";
		ok+="Via: "+via+"\r\n";
		ok+="From: "+from+"\r\n";
		ok+="To: "+to+";tag=1945341956\r\n";
		ok+="Call-ID: "+call_id+"\r\n";
		ok+="CSeq: "+c_seq+"\r\n";
		ok+="Allow: INVITE, ACK, BYE, CANCEL, OPTIONS, MESSAGE, SUBSCRIBE, NOTIFY, INFO\r\n";
		ok+="Accept: application/sdp\r\n";
		ok+="User-Agent: Linphone/3.3.2 (eXosip2/3.3.0)\r\n";
		ok+="Content-Length: 0\r\n";
		return ok;

	}
	
};
