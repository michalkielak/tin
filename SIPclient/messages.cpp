#include <iostream>
#include <cstring>
using namespace std;

class Messages{
public:
string registerMsg;
string registerAuthMsg;
string msg3, msg2;
string invite;

void init()
{
	registerMsg = "REGISTER sip:194.29.169.4 SIP/2.0\r\n";
	registerMsg+="Via: SIP/2.0/UDP 10.0.2.15:8060;rport;branch=z9hG4bK1606792150\r\n";
	registerMsg+="Transport: UDP\r\n";
	registerMsg+="Sent-by Address: 10.0.2.15\r\n";
	registerMsg+="Sent-by port: 8060\r\n";
	registerMsg+="RPort: rport\r\n";
	registerMsg+="Branch: z9hG4bK1606792150\r\n";
	registerMsg+="From: <sip:stud5@194.29.169.4>;tag=697264525\r\n";
	registerMsg+="SIP from address: sip:stud5@194.29.169.4\r\n";
	registerMsg+="SIP from address User Part: stud5\r\n";
	registerMsg+="SIP from address Host Part: 194.29.169.4\r\n";
	registerMsg+="SIP from address Host Port: 8060\r\n";
	registerMsg+="SIP tag: 697264525\r\n";
	registerMsg+="To: <sip:stud5@194.29.169.4>\r\n";
	registerMsg+="SIP to address: sip:stud5@194.29.169.4\r\n";
	registerMsg+="SIP to address User Part: stud5\r\n";
	registerMsg+="SIP to address Host Part: 194.29.169.4\r\n";
	registerMsg+="SIP to address Host Port: 8060\r\n";
	registerMsg+="Call-ID: 2082372650\r\n";
	registerMsg+="CSeq: 2 REGISTER\r\n";
	registerMsg+="Contact: <sip:stud5@10.0.2.15>\r\n";
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

	registerAuthMsg = "REGISTER sip:194.29.169.4 SIP/2.0\r\n";
	registerAuthMsg+="Via: SIP/2.0/UDP 10.0.2.15:8060;rport;branch=z9hG4bK1606792150\r\n";
	registerAuthMsg+="Transport: UDP\r\n";
	registerAuthMsg+="Sent-by Address: 10.0.2.15\r\n";
	registerAuthMsg+="Sent-by port: 8060\r\n";
	registerAuthMsg+="RPort: rport\r\n";
	registerAuthMsg+="Branch: z9hG4bK1606792150\r\n";
	registerAuthMsg+="From: <sip:stud5@194.29.169.4>;tag=697264525\r\n";
	registerAuthMsg+="SIP from address: sip:stud5@194.29.169.4\r\n";
	registerAuthMsg+="SIP from address User Part: stud5\r\n";
	registerAuthMsg+="SIP from address Host Part: 194.29.169.4\r\n";
	registerAuthMsg+="SIP from address Host Port: 8060\r\n";
	registerAuthMsg+="SIP tag: 697264525\r\n";
	registerAuthMsg+="To: <sip:stud5@194.29.169.4>\r\n";
	registerAuthMsg+="SIP to address: sip:stud5@194.29.169.4\r\n";
	registerAuthMsg+="SIP to address User Part: stud5\r\n";
	registerAuthMsg+="SIP to address Host Part: 194.29.169.4\r\n";
	registerAuthMsg+="SIP to address Host Port: 8060\r\n";
	registerAuthMsg+="Call-ID: 2082372650\r\n";
	registerAuthMsg+="CSeq: 2 REGISTER\r\n";
	registerAuthMsg+="Contact: <sip:stud5@10.0.2.15>\r\n";
	//registerAuthMsg+="Contact-URI: sip:stud5@87.205.195.253:8060\r\n";
	/*registerAuthMsg+="Authentication Scheme: Digest\r\n";
	registerAuthMsg+="username=\"stud5\"\r\n";
	registerAuthMsg+="realm=\"tele.pw.edu.pl\"\r\n";*/

msg3 = "REGISTER sip:194.29.169.4 SIP/2.0\r\n";
msg3+="CSeq: 2 REGISTER\r\n";
msg3+="Via: SIP/2.0/UDP 10.0.2.15:8060;branch=z9hG4bK32366531-99e1-de11-8845-080027608325;rport\r\n";
msg3+="User-Agent: Ekiga/3.2.5\r\n";
/*msg3+="From: <sip:stud5@194.29.169.4>;tag=d60e6131-99e1-de11-8845-080027608325\r\n";
msg3+="Call-ID: e4ec6031-99e1-de11-8845-080027608325@vvt-laptop\r\n";
msg3+="To: <sip:stud5@194.29.169.4>\r\n";
msg3+="Contact: <sip:stud5@194.29.169.4>;q=1\r\n";
msg3+="Allow: INVITE,ACK,OPTIONS,BYE,CANCEL,SUBSCRIBE,NOTIFY,REFER,MESSAGE,INFO,PING\r\n";
msg3+="Expires: 3600\r\n";
msg3+="Content-Length: 0\r\n";
msg3+="Max-Forwards: 70\r\n";*/

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

	invite = "INVITE sip:toto@192.168.47.146 SIP/2.0\r\n";
	invite+= "Via: SIP/2.0/UDP pc33.server1.com;branch=z9hG4bK776asdhds Max-Forwards: 70\r\n";
	invite+= "To: toto <sip:toto@192.168.47.146>\r\n";
	invite+= "From: user1 <sip:user1@server1.com>;tag=1928301774\r\n";
	invite+= "Call-ID: a84b4c76e66710@pc33.server1.com\r\n";
	invite+= "CSeq: 314159 INVITE\r\n";
	invite+= "Contact: <sip:user1@pc33.server1.com>\r\n";
	invite+= "Content-Type: application/sdp\r\n";
	invite+= "Content-Length: 0\r\n";
}

string getRegisterMsg()
{
	//return msg2;
	return registerMsg;
}

string getRegisterAuthMsg(string nonce)
{
	/*registerAuthMsg+="nonce=\"";
	registerAuthMsg+=nonce;
	registerAuthMsg+="\"\r\n";
	registerAuthMsg+="uri=\"sip:194.29.169.4:8060\"\r\n";
	registerAuthMsg+="response=\"11de87eb98123dfd2ae9ed3182c2cb2f\"\r\n";
	registerAuthMsg+="algorithm=MD5\r\n";*/
	registerAuthMsg+="Authorization: Digest username=\"stud5\", realm=\"tele.pw.edu.pl\", nonce=\"";
	registerAuthMsg+=nonce;
	registerAuthMsg+="\", uri=\"sip:194.29.169.4\", response=\"11de87eb98123dfd2ae9ed3182c2cb2f\", algorithm=MD5\r\n";
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
	return invite;
}

};
