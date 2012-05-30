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
string myIp, otherIp, serverIp, myLogin, otherLogin;
string contentSize;

Messages(string myLogin, string myIp, string otherLogin, string otherIp, string serverIp)
{
	this->myIp = myIp;
	this->otherIp = otherIp;
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

void init()
{
	
/*msg3 = "REGISTER sip:194.29.169.4 SIP/2.0\r\n";
msg3+="CSeq: 2 REGISTER\r\n";
msg3+="Via: SIP/2.0/UDP 10.0.2.15:8060;branch=z9hG4bK32366531-99e1-de11-8845-080027608325;rport\r\n";
msg3+="User-Agent: Ekiga/3.2.5\r\n";
msg3+="From: <sip:stud5@194.29.169.4>;tag=d60e6131-99e1-de11-8845-080027608325\r\n";
msg3+="Call-ID: e4ec6031-99e1-de11-8845-080027608325@vvt-laptop\r\n";
msg3+="To: <sip:stud5@194.29.169.4>\r\n";
msg3+="Contact: <sip:stud5@194.29.169.4>;q=1\r\n";
msg3+="Allow: INVITE,ACK,OPTIONS,BYE,CANCEL,SUBSCRIBE,NOTIFY,REFER,MESSAGE,INFO,PING\r\n";
msg3+="Expires: 3600\r\n";
msg3+="Content-Length: 0\r\n";
msg3+="Max-Forwards: 70\r\n";

msg2 = "REGISTER sip:194.29.169.4 SIP/2.0\r\n";
msg2+="CSeq: 1 REGISTER\r\n";
msg2+="Via: SIP/2.0/UDP 10.0.2.15:8060;branch=z9hG4bK32366531-99e1-de11-8845-080027608325;rport\r\n";
msg2+="User-Agent: Ekiga/3.2.5\r\n";
msg2+="From: <sip:stud5@194.29.169.4>;tag=d60e6131-99e1-de11-8845-080027608325\r\n";
msg2+="Call-ID: e4ec6031-99e1-de11-8845-080027608325@vvt-laptop\r\n";
msg2+="To: <sip:stud5@194.29.169.4>\r\n";
msg2+="Contact: <sip:stud5@87.205.195.253:64381>;q=1\r\n";
msg2+="Allow: INVITE,ACK,OPTIONS,BYE,CANCEL,SUBSCRIBE,NOTIFY,REFER,MESSAGE,INFO,PING\r\n";
msg2+="Expires: 3600\r\n";
msg2+="Content-Length: 0\r\n";
msg2+="Max-Forwards: 70\r\n";

	invite = "INVITE sip:"+myLogin+"@"+otherIp+" SIP/2.0\r\n";
	invite+= "Via: SIP/2.0/UDP "+myIp+";branch=z9hG4bK776asdhds Max-Forwards: 70\r\n";
	invite+= "To: "+myLogin+" <sip:myLogin@"+otherIp+">\r\n";
	invite+= "From: user1 <sip:user1@"+myIp+">;tag=1928301774\r\n";
	invite+= "Call-ID: a84b4c76e66710@pc33.server1.com\r\n";
	invite+= "CSeq: 314159 INVITE\r\n";
	invite+= "Contact: <sip:user1@"+myIp+">\r\n";
	invite+= "Content-Type: application/sdp\r\n";
	invite+= "Content-Length: 0\r\n";

	invite2 = "INVITE sip:myLogin@"+otherIp+" SIP/2.0\r\n";
	invite2+="Via: SIP/2.0/UDP "+myIp+":8060;rport;branch=z9hG4bK1146215919\r\n";
	invite2+="From: <sip:user1@"+myIp+">;tag=877947723\r\n";
	invite2+="To: <sip:myLogin@"+otherIp+">\r\n";
	invite2+="Call-ID: 140398538\r\n";
	invite2+="CSeq: 20 INVITE\r\n";
	invite2+="Contact: <sip:user1@"+myIp+">\r\n";
	invite2+="Content-Type: application/sdp\r\n";
	invite2+="Allow: INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, NOTIFY, MESSAGE, SUBSCRIBE, INFO\r\n";
	invite2+="Max-Forwards: 70\r\n";
	invite2+="User-Agent: Linphone/3.3.2 (eXosip2/3.3.0)\r\n";
	invite2+="Subject: Phone call\r\n";
	invite2+="Content-Length:   400\r\n";
	invite2+=" \r\n";
	invite2+="v=0\r\n";
	invite2+="o=myLogin 123456 654321 IN IP4 10.0.2.15\r\n";
	invite2+="s=A conversation\r\n";
	invite2+="c=IN IP4 10.0.2.15\r\n";
	invite2+="t=0 0\r\n";
	invite2+="m=audio 7078 RTP/AVP 112 111 110 3 0 8 101\r\n";
	invite2+="a=rtpmap:112 speex/32000/1\r\n";
	invite2+="a=fmtp:112 vbr=on\r\n";
	invite2+="a=rtpmap:111 speex/16000/1\r\n";
	invite2+="a=fmtp:111 vbr=on\r\n";
	invite2+="a=rtpmap:110 speex/8000/1\r\n";
	invite2+="a=fmtp:110 vbr=on\r\n";
	invite2+="a=rtpmap:3 GSM/8000/1\r\n";
	invite2+="a=rtpmap:0 PCMU/8000/1\r\n";
	invite2+="a=rtpmap:8 PCMA/8000/1\r\n";
	invite2+="a=rtpmap:101 telephone-event/8000/1\r\n";
	invite2+="a=fmtp:101 0-11\r\n";*/



}

string getRegisterMsg()
{
	registerMsg = "REGISTER sip:"+ serverIp +" SIP/2.0\r\n";
	registerMsg+="Via: SIP/2.0/UDP "+ myIp +":8060;rport;branch=z9hG4bK1606792150\r\n";
	/*registerMsg+="Transport: UDP\r\n";
	registerMsg+="Sent-by Address: "+ myIp +"\r\n";
	registerMsg+="Sent-by port: 8060\r\n";
	registerMsg+="RPort: rport\r\n";
	registerMsg+="Branch: z9hG4bK1606792150\r\n";*/
	registerMsg+="From: <sip:"+ myLogin +"@"+ serverIp +">;tag=697264525\r\n";
	/*registerMsg+="SIP from address: sip:"+ myLogin +"@"+ serverIp +"\r\n";
	registerMsg+="SIP from address User Part: "+ myLogin +"\r\n";
	registerMsg+="SIP from address Host Part: "+ serverIp +"\r\n";
	registerMsg+="SIP from address Host Port: 8060\r\n";
	registerMsg+="SIP tag: 697264525\r\n";*/
	registerMsg+="To: <sip:"+ myLogin +"@"+ serverIp +">\r\n";
	/*registerMsg+="SIP to address: sip:"+ myLogin +"@"+ serverIp +"\r\n";
	registerMsg+="SIP to address User Part: "+ myLogin +"\r\n";
	registerMsg+="SIP to address Host Part: "+ serverIp +"\r\n";
	registerMsg+="SIP to address Host Port: 8060\r\n";*/
	registerMsg+="Call-ID: 2082372650\r\n";
	registerMsg+="CSeq: 1 REGISTER\r\n";
	registerMsg+="Contact: <sip:"+ myLogin +"@"+ myIp +":8060;line=171085286da3c77>\r\n";
	/*registerMsg+="Authorization: Digest username=\"stud5\", realm=\"tele.pw.edu.pl\", nonce=\"809efae20abc344f92013866e39d4e4736417119\", uri=\"sip:194.29.169.4:8060\", response=\"11de87eb98123dfd2ae9ed3182c2cb2f\", algorithm=MD5\r\n";
	registerMsg+="Authentication Scheme: Digest\r\n";
	registerMsg+="username=\"stud5\"\r\n";
	registerMsg+="realm=\"tele.pw.edu.pl\"\r\n";
	registerMsg+="nonce=\"809efae20abc344f92013866e39d4e4736417119\"\r\n";
	registerMsg+="uri=\"sip:194.29.169.4:8060\"\r\n";
	registerMsg+="response=\"11de87eb98123dfd2ae9ed3182c2cb2f\"\r\n";
	registerMsg+="algorithm=MD5\r\n";*/
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
	/*registerAuthMsg+="Transport: UDP\r\n";
	registerAuthMsg+="Sent-by Address: "+ myIp +"\r\n";
	registerAuthMsg+="Sent-by port: 8060\r\n";
	registerAuthMsg+="RPort: rport\r\n";
	registerAuthMsg+="Branch: z9hG4bK1606792150\r\n";*/
	registerAuthMsg+="From: <sip:"+ myLogin +"@"+ serverIp +">;tag=697264525\r\n";
	/*registerAuthMsg+="SIP from address: sip:"+ myLogin +"@"+ serverIp +"\r\n";
	registerAuthMsg+="SIP from address User Part: "+ myLogin +"\r\n";
	registerAuthMsg+="SIP from address Host Part: "+ serverIp +"\r\n";
	registerAuthMsg+="SIP from address Host Port: 8060\r\n";
	registerAuthMsg+="SIP tag: 697264525\r\n";*/
	registerAuthMsg+="To: <sip:"+ myLogin +"@"+ serverIp +">\r\n";
	/*registerAuthMsg+="SIP to address: sip:"+ myLogin +"@"+ serverIp +"\r\n";
	registerAuthMsg+="SIP to address User Part: "+ myLogin +"\r\n";
	registerAuthMsg+="SIP to address Host Part: "+ serverIp +"\r\n";
	registerAuthMsg+="SIP to address Host Port: 8060\r\n";*/
	registerAuthMsg+="Call-ID: 2082372650\r\n";
	registerAuthMsg+="CSeq: 2 REGISTER\r\n";
	registerAuthMsg+="Contact: <sip:"+ myLogin +"@"+ myIp +":8060;line=0997072b3c9b658>\r\n";
	/*registerAuthMsg+="nonce=\"";
	registerAuthMsg+=nonce;
	registerAuthMsg+="\"\r\n";
	registerAuthMsg+="uri=\"sip:194.29.169.4:8060\"\r\n";
	registerAuthMsg+="response=\"11de87eb98123dfd2ae9ed3182c2cb2f\"\r\n";
	registerAuthMsg+="algorithm=MD5\r\n";*/
	registerAuthMsg+="Authorization: Digest username=\"stud5\", realm=\"tele.pw.edu.pl\", nonce=\"";
	registerAuthMsg+=nonce;
	registerAuthMsg+="\", uri=\"sip:194.29.169.4\", response=\"" +response+ "\", algorithm=MD5\r\n";
	registerAuthMsg+="Max-Forwards: 70\r\n";
	registerAuthMsg+="User-Agent: Linphone/3.3.2 (eXosip2/3.3.0)\r\n";
	registerAuthMsg+="Expires: 3600\r\n";
	registerAuthMsg+="Content-Length: 0\r\n";

	msg3+="Authorization: Digest username=\"stud5\", realm=\"tele.pw.edu.pl\",nonce=\"";
	msg3+=nonce;
	msg3+="\", uri=\"sip:194.29.169.4\", algorithm=MD5,response=\"6c13de87f9cde9c44e95edbb68cbdea9\"\r\n";
	msg3+="From: <sip:stud5@194.29.169.4>;tag=d60e6131-99e1-de11-8845-080027608325\r\n";
	msg3+="Call-ID: e4ec6031-99e1-de11-8845-080027608325@vvt-laptop\r\n";
	msg3+="To: <sip:stud5@194.29.169.4>\r\n";
	msg3+="Contact: <sip:stud5@87.205.195.253:64381>;q=1\r\n";
	msg3+="Allow: INVITE,ACK,OPTIONS,BYE,CANCEL,SUBSCRIBE,NOTIFY,REFER,MESSAGE,INFO,PING\r\n";
	msg3+="Expires: 3600\r\n";
	msg3+="Content-Length: 0\r\n";
	msg3+="Max-Forwards: 70\r\n";


	return registerAuthMsg;
	//return msg3;
}

string getInviteMsg()
{
	invite3 ="INVITE sip:"+otherLogin+"@"+serverIp+":8060 SIP/2.0\r\n";
	invite3+="Via: SIP/2.0/UDP "+myIp+":8060;rport;branch=z9hG4bK1858780968\r\n";//!
	invite3+="From: <sip:"+myLogin+"@"+serverIp+":8060>;tag=1945341956\r\n";
	invite3+="To: <sip:"+otherLogin+"@"+otherIp+":8060>\r\n";
	invite3+="Call-ID: 1411118635\r\n";
	invite3+="CSeq: 20 INVITE\r\n";
	invite3+="Contact: <sip:"+myLogin+"@"+myIp+":8060>\r\n";//!
	invite3+="Content-Type: application/sdp\r\n";
	invite3+="Allow: INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, NOTIFY, MESSAGE, SUBSCRIBE, INFO\r\n";
	invite3+="Max-Forwards: 70\r\n";
	invite3+="User-Agent: Linphone/3.3.2 (eXosip2/3.3.0)\r\n";
	invite3+="Subject: Phone call\r\n";
	invite3+="Content-Length:   "+ contentSize +"\r\n";//from 405+4
	invite3+="\r\n";
	invite3+="v=0\r\n";
	invite3+="o="+myLogin+" 123456 654321 IN IP4 "+myIp+"\r\n";//!
	invite3+="s=A conversation\r\n";
	invite3+="c=IN IP4 "+myIp+"\r\n";//!
	invite3+="t=0 0\r\n";
	invite3+="m=audio 7078 RTP/AVP 112 111 110 3 0 8 101\r\n";
	invite3+="a=rtpmap:112 speex/32000/1\r\n";
	invite3+="a=fmtp:112 vbr=on\r\n";
	invite3+="a=rtpmap:111 speex/16000/1\r\n";
	invite3+="a=fmtp:111 vbr=on\r\n";
	invite3+="a=rtpmap:110 speex/8000/1\r\n";
	invite3+="a=fmtp:110 vbr=on\r\n";
	invite3+="a=rtpmap:3 GSM/8000/1\r\n";
	invite3+="a=rtpmap:0 PCMU/8000/1\r\n";
	invite3+="a=rtpmap:8 PCMA/8000/1\r\n";
	invite3+="a=rtpmap:101 telephone-event/8000/1\r\n";
	invite3+="a=fmtp:101 0-11\r\n";

	//return invite;
	//return invite2;
	return invite3;
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
