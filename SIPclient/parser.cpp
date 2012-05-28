#include <stdlib.h>
#include <sys/time.h>
#include <osip2/osip.h>

//wiadomosc z ghuba
char* msg3 = "BYE sip:michal@192.168.46.88:8060 SIP/2.0\r\nVia: SIP/2.0/UDP 192.168.47.25:8060;rport;branch=z9hG4bK1394956433\r\nFrom: <sip:tin@192.168.47.25>;tag=1945341956\r\nTo: <sip:michal@192.168.46.88:8060>;tag=32391\r\nCall-ID: 1411118635\r\nCSeq: 21 BYE\r\nContact: <sip:tin@192.168.47.25:8060>\r\nMax-Forwards: 70\r\nUser-Agent: Linphone/3.3.2 (eXosip2/3.3.0)\r\nContent-Length: 0\r\n";

/* argumenty:
	msg - wiadomosc
	len - dlugosc wiadomosci
	code - zwraca kod wiadomosci
	contact_ip - zwraca adres ip z naglowka contact
*/   
int parseMsg(char *msg, size_t len, int *code, char **contact_ip, char **tag) {
	parser_init();
	osip_message_t *message;
	osip_message_init(&message);
	osip_message_parse(message, msg, len);
	
	osip_contact_t *contact_header;
	osip_message_get_contact(message, 0, &contact_header);
	
	osip_uri_t *contact_uri = osip_contact_get_url(contact_header);
	*contact_ip = osip_strdup(osip_uri_get_host(contact_uri));
	
	*code = osip_message_get_status_code(message); 
	
	if (*code == 200) {
		osip_uri_param_t *toTag;
		osip_to_t *to = osip_message_get_to(message);
		osip_to_get_tag(to, &toTag);
		*tag = osip_strdup(toTag->gvalue);
	}
	
	//printf("(L)%s\n",*tag);
	return 0;

}

int getOptions(char *msg, size_t len, char **via, char** from, char ** to, char **call_id, char **c_seq) {
	parser_init();
	osip_message_t *message;
	osip_message_init(&message);
	osip_message_parse(message, msg, len);
	
	osip_via_t *v;
	osip_message_get_via(message, 0, &v);
	osip_via_to_str(v, via);

	osip_from_t *f;
	f = osip_message_get_from(message);
	osip_from_to_str(f, from);

	osip_to_t *t;
	t = osip_message_get_to(message);
	osip_to_to_str(t, to);
	
	osip_call_id_t * id;
	id = osip_message_get_call_id(message);
	osip_call_id_to_str(id, call_id);

	osip_cseq_t *seq;
	seq = osip_message_get_cseq(message);
	osip_cseq_to_str(seq, c_seq);

	return 0;
}
/*
int main (void) {
	size_t length = strlen(msg3);
	int status = 0;
	char *contact_ip = NULL;
	char *via, *from, *to, *call_id, *c_seq;
	//parseMsg(msg3, length, &status, &contact_ip, &tag1);

	//printf("%s, %d, %s\n", contact_ip, status, tag1);	
	getOptions(msg3, length, &via, &from, &to, &call_id, &c_seq);
	printf("%s\n%s\n%s\n%s\n%s\n", via, from, to, call_id, c_seq);
		
	return 0;
}*/
