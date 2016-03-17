#include "include/tcp.h"

#define BACKLOG 10
#define MAX_BUF_LEN 4096

static int tcp_raw_recv(int socket, char *buffer);
static int to_data_serialize(to_packet_t * packet);
static int to_data_deserialize(to_packet_t * packet);

char * to_tcp_packet_strtype(to_packet_type type){
	switch(type){
	case PACKET_UPDATE:
		return "UPDATE";

	case PACKET_ACK:
		return "ACK";

	case PACKET_NACK:
		return "NACK";

	case PACKET_CRC:
		return "CRC";

	default:
		return "UNKNOWN";
	}
}

to_packet_t * to_tcp_prep_packet(void){
	to_packet_t * p = malloc(sizeof(to_packet_t));
	if(!p) return NULL;
	p->socket = -1;
	p->raw_data = NULL;
	p->raw_data_len = 0;
	memset(p->obj_path, 0, OBJ_PATH_LEN);
	p->obj_data = NULL;
	p->obj_data_len = 0;
	p->crc = -1;

	return p;
}

void to_tcp_packet_destroy(to_packet_t **packet){
	if((*packet)->raw_data) free((*packet)->raw_data);
	if((*packet)->obj_data) free((*packet)->obj_data);
	close((*packet)->socket);
	free(*packet);
	*packet = NULL;
}

int to_tcp_listen(char const * ip, char const * port){
	struct addrinfo servSide, *servInfo, *t;
	int sock;
	
	memset(&servSide, 0, sizeof servSide);
	servSide.ai_family = AF_UNSPEC;
	servSide.ai_socktype = SOCK_STREAM;
	servSide.ai_flags = AI_PASSIVE;
        
	if(getaddrinfo(ip, port, &servSide, &servInfo) != 0){
		to_log_err("Could not get server's info");
		sock = -1;
		goto ERROR;
	}
	for(t = servInfo; t != NULL; t = t->ai_next){
		if((sock = socket(t->ai_family, t->ai_socktype, t->ai_protocol)) == -1){
			to_log_err("Socket problem");
			continue;
		}
		
		int yes = 1;
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		
		if(bind(sock, t->ai_addr, t->ai_addrlen) == -1){
			close(sock);
			to_log_err("Bind returned error");
			continue;
		}
		break;
	}
	
	if (t == NULL)  {
		to_log_err("Failed to bind to server");
		sock = -1;
		goto ERROR;
	}
	
	if (listen(sock, BACKLOG) == -1) {
		to_log_err("listen() returned error ");
		sock = -1;
		goto ERROR;
	}
	
	LOG_LEVEL0("Server listening");
	
 ERROR:
	freeaddrinfo(servInfo);
	return sock;
}

int to_tcp_accept(int socket){
	struct sockaddr_in client_ip;
	char client_ip_s[INET_ADDRSTRLEN] = {0};
	socklen_t salen = 0;

	int req_sock = accept(socket, (struct sockaddr *)&client_ip, &salen);
	if(0 > req_sock){
		to_log_err("Error accepting connection: [%s]", strerror(errno));
		return req_sock;
	}else{
		/* Show us who is connecting */
		inet_ntop(AF_INET, &client_ip.sin_addr, client_ip_s, sizeof client_ip_s);
		LOG_LEVEL1("Accepting connection from [%s]", client_ip_s);

		return req_sock;
	}
}

int to_tcp_remote_connect(char const * ip, char const * port){
	
	struct addrinfo servSide, *servInfo, *t;
	int sockfd;
	char addr[INET_ADDRSTRLEN];
	
	//configure the net stuff first
	memset(&servSide, 0, sizeof servSide);
	servSide.ai_family = AF_UNSPEC;
	servSide.ai_socktype = SOCK_STREAM;
	servSide.ai_flags = AI_PASSIVE;

	if(getaddrinfo(ip, port, &servSide, &servInfo) != 0){
		to_log_warn("Could not get remote host's info");
		return -1; //not sure if it's a terminal offence
	}
	
	for(t = servInfo; t != NULL; t = t->ai_next){
		if((sockfd = socket(t->ai_family, t->ai_socktype, t->ai_protocol))== -1){
			to_log_warn("Problem obtainig socket");
			continue;
		}    
		
		if(connect(sockfd, t->ai_addr, t->ai_addrlen) == -1){
			close(sockfd);
			to_log_warn("connect() returned error");
			continue;
		}
		
		break;
	}
	
	if(t == NULL){
		to_log_err("Failed to connect to [%s]", ip);
		freeaddrinfo(servInfo);
		return -1;
	}

	/* Who did we connect to? */
	struct sockaddr_in *ipaddr = (struct sockaddr_in *)t->ai_addr;
	inet_ntop(t->ai_family, &ipaddr->sin_addr, addr, sizeof addr);
	LOG_LEVEL0("Connected to %s\n", addr);
	
	freeaddrinfo(servInfo); 

	return sockfd;
}

int to_tcp_send_packet(to_packet_t * packet){
  	
	int n, total_bytes = 0;
	int bytes_left = 0;

	LOG_LEVEL1("Serializing data");
	if(to_data_serialize(packet) < 0){
		to_log_err("Error serializing data");
		return -1;
	}

	bytes_left = packet->raw_data_len;
	
	/* send all data to slave */
	while(total_bytes < packet->raw_data_len){
		
		n = send(packet->socket, packet->raw_data, bytes_left, 0);
		if(n == -1){
			break;
		}
		total_bytes += n;
		bytes_left -= n;
	}
	
	LOG_LEVEL1("Total bytes sent %d\n", total_bytes);
	
	return total_bytes;
}

to_packet_t * to_tcp_read_packet(int socket, bool wait){

	int len;
	char raw_data[MAX_BUF_LEN];
	struct timeval timeout;
	to_packet_t * packet;
	
	packet = to_tcp_prep_packet();
	if(!packet) return NULL;


	if(wait){
		/* Init the struct to keep valgrind quiet */
		memset(&timeout, 0, sizeof(timeout));
		timeout.tv_sec = 0;
		timeout.tv_usec = 5000;
		setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	}
	
	len = tcp_raw_recv(socket, raw_data);
	if(!(len > 0)) return NULL;

	LOG_LEVEL2("Read [%d] bytes", len);

	packet->socket = socket;

	/* raw_data is already correctly '\0' terminated by @tcp_raw_recv */
	packet->raw_data = malloc(sizeof(char) * len);
	if(!packet->raw_data) return NULL;

	memcpy(packet->raw_data, raw_data, len);
	packet->raw_data_len = len;

	if(to_data_deserialize(packet)){
		to_log_err("Failure deserializing response");
		to_tcp_packet_destroy(&packet);
		return NULL;
	}
	
	return packet;
}

static int tcp_raw_recv(int socket, char *buffer){
	int num;
	char byte;
	for(int i = 1; i < MAX_BUF_LEN; i++){
		num = recv(socket, &byte, 1, 0);
		if(1 == num){
			if((byte == '\x1f') && (*(buffer - 1) == '\x1f')){
				/* read the last byte and terminater it */
				*(buffer++) = byte;
				*buffer = '\0';
				return i;
			}
			/* copy and continue */
			*(buffer++) = byte;
		}else if(0 == num){
			/* reciever disconnected? */
			return i;
		}else{
			to_log_err("Problem in recv(): ERRNO(%s)", strerror(errno));
		}
	}
	
	return -1; /* something weird happened */
}

/* Packet types
           |******|****|**********|****|********|****|*********|****|*****|******|
 UPDATE    | type | \n | FILENAME | \n | LENGHT | \n | PAYLOAD | \n | CRC | \n\n |
           |******|****|**********|****|********|****|*********|****|*****|******|
	   |******|******|
 ACK\NACK  | type | \n\n | 
	   |******|******|
	   |******|****|**********|****|*****|******|
 CRC	   | type | \n | FILENAME | \n | CRC | \n\n |
	   |******|****|**********|****|*****|******|
*/

#define COMMA() (*(r_data + here++) =  '\x1f')
#define DOT()					\
	do{					\
		*(r_data + here++) =  '\x1f';	\
		*(r_data + here++) =  '\x1f';	\
	}while(0);				\

static int to_data_serialize(to_packet_t * packet){
	
	int here = 0;           /* currrrent location pointer */
	size_t packet_len = 0;  /* overal packet length */
	char *r_data;           /* alias pointer for packet->raw_data */
	uint8_t chsum = 0;      /* For CRC checksum */
	
	switch(packet->packet_type){

	case PACKET_UPDATE:

		packet_len = strlen(packet->obj_path) + packet->obj_data_len;

		/*
		  1 for type, 1 for object path, 1 for data,
		  1 byte for crc result, 2 for '\x1f\x1f'
		  and sizeof(size_t) for len
		*/
		LOG_LEVEL2("Allocating memory for UPDATE packet");
		r_data = packet->raw_data = malloc(8 + sizeof(size_t) + packet_len * sizeof(char));
		if(!r_data){
			to_log_err("Failed to allocate memory for UPDATE packet");
			return -1;
		}

		/* First type */
		*(r_data + here++) =  packet->packet_type;
		COMMA();
		
		/* ...then file name */
		LOG_LEVEL2("Copying obj file path");
		memcpy(r_data + here, packet->obj_path, strlen(packet->obj_path));
		here += strlen(packet->obj_path);
		COMMA();
		
		/* 
		   then the sync data length and '\x1f'
		*/
		for(int i = (sizeof(size_t) - 1); i >= 0; i--){
			
			*(r_data + here) = (packet->obj_data_len >> i*8) & 0xff;
			here++;
		}
		COMMA();

		/* then the actual data */
		memcpy(r_data + here, packet->obj_data, packet->obj_data_len);
		here += packet->obj_data_len;
		COMMA();
		
		/* and then the crc cherry on top */
		LOG_LEVEL2("Calculating checksum");
		chsum = crc(packet->obj_data, packet->obj_data_len);
		*(r_data + here++) = (char)chsum;
		LOG_LEVEL2("Checksum: [%d]", chsum);
		
		/* terminate it with two of those */
		DOT();

		packet->raw_data_len = here;
		
		return here;

	case PACKET_CRC:

		LOG_LEVEL2("Allocating memory for CRC check packet");
		/* Path len + CRC len + 4 for separators */
		r_data = packet->raw_data = malloc(5 + strlen(packet->obj_path) * sizeof(char));
		if(!r_data){
			to_log_err("Failed to allocate memory for CRC packet");
			return -1;
		}

		/* Copy packet type */
		*(r_data + here++) =  packet->packet_type;
		COMMA();

		LOG_LEVEL2("Copying obj file path");
		memcpy(r_data + here, packet->obj_path, strlen(packet->obj_path));
		here += strlen(packet->obj_path);
		COMMA();
		
		LOG_LEVEL2("Calculating checksum");
		chsum = crc(packet->obj_data, packet->obj_data_len);
		*(r_data + here++) = (char)chsum;
		LOG_LEVEL2("Checksum: [%d]", chsum);

		DOT();

		packet->raw_data_len = here;
		
		return here;

	case PACKET_ACK:
	case PACKET_NACK:

		LOG_LEVEL2("Allocating memory for ACK/NACK packet");
		r_data = packet->raw_data = malloc(3);
		if(!r_data){
			to_log_err("Failed to allocate memory for ACK/NACK packet");
			return -1;
		}
		/* Just copy packet type */
		*(r_data + here++) =  packet->packet_type;

		DOT();
		
		packet->raw_data_len = here;
		
		return here;

	default:
		to_log_err("Error serializing unknown packet type");
		return -1;
	}
	
	return -1;
}
#undef COMMA
#undef DOT

/* Stop it from while-ing indefinitely in case of missing '\x1f' */
#define FIND_COMMA() while(*(here++) != '\x1f' && (here - packet->raw_data) < packet->raw_data_len);

static int to_data_deserialize(to_packet_t * packet){

	size_t obj_len = 0;
	char *there;
	char *here = there = packet->raw_data;

	/* 
	   get type 
	   its only 1 byte
	*/
	packet->packet_type = *here;
	LOG_LEVEL1("Received [%s] packet", to_tcp_packet_strtype(packet->packet_type));
	FIND_COMMA();

	switch(packet->packet_type){
	case PACKET_UPDATE:

		/* get fileneame */
		there = here;
		FIND_COMMA();
		if((here - there) < 1) return -1;
		
		memcpy(packet->obj_path, there, here - there - 1);
		packet->obj_path[here - there - 1] = '\0';
		LOG_LEVEL0("File recved [%s]", packet->obj_path);
		/* update the pointer */
		there = here;
		
		/* get len */
		FIND_COMMA();
		uint8_t a_len[sizeof(size_t)] = { 0 };
		
		memcpy(a_len, there, here - there - 1);
		for(int i = sizeof(size_t) - 1; i >= 0; i--){
			obj_len |= (a_len[sizeof(size_t)-1 - i] & 0xff) << i*8;
		}
		
		if(obj_len < 1) return -1;
		
		/* got the len so lets alloc the space for it */
		packet->obj_data_len = obj_len;
		packet->obj_data = malloc(1 + obj_len * sizeof(char));
		if(!packet->obj_data) return -1;
	
		there = here;
		/* get the sync data */
		FIND_COMMA();
		if(here - there < 1) return -1; 
		memcpy(packet->obj_data, there, obj_len);
		packet->obj_data[obj_len] = '\0';
		
		/* all is left is crc */
		there = here;
		FIND_COMMA();
		/* crc should only be 1 byte */
		packet->crc = (*there) & 0xff;

		return 0;

	case PACKET_ACK:
	case PACKET_NACK:
		/* Its ACK/NACK so not much to do now */
		return 0;

	case PACKET_CRC:
		/* get filename */
		FIND_COMMA();
		if((here - there) < 1) return -1;
		
		memcpy(packet->obj_path, there, here - there - 1);
		packet->obj_path[here - there - 1] = '\0';
		
		/* update the pointer */
		there = here;
		
		/* all is left is crc */
		FIND_COMMA();
		/* crc should only be 1 byte */
		packet->crc = (*there) & 0xff;

		break;
	}
	
	return 0;
}

#undef FIND_COMMA
