#include "parser.h"
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

int getIp(char *msg, size_t len, char **ip) {
	parser_init();
	osip_message_t *message;
	osip_message_init(&message);
	osip_message_parse(message, msg, len);
	
	osip_via_t *v;
	osip_message_get_via(message, 1, &v);
	*ip = osip_strdup(osip_via_get_host(v));
	
	osip_message_free(message);
	
	return 0;
}

char* getNonce(char* msg, size_t len, char **realm) {
	parser_init();
	osip_message_t *message;
	osip_message_init(&message);
	osip_message_parse(message, msg, len);
	
	char *nonce = NULL;
	osip_proxy_authenticate_t * proxy_auth;
	if (osip_message_get_proxy_authenticate(message, 0, &proxy_auth) == 0 ) 
		nonce = osip_strdup(osip_proxy_authenticate_get_nonce(proxy_auth));
	
	
	osip_www_authenticate_t *www_auth;
	if (osip_message_get_www_authenticate(message, 0, &www_auth) == 0)
	{
		nonce = osip_strdup(osip_www_authenticate_get_nonce(www_auth));
		*realm = osip_strdup(www_auth->realm);
	}
	osip_message_free(message);
	
	return nonce;
}