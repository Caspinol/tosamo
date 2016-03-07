#include "include/tcp.h"

#define BACKLOG 10
#define MAX_BUF_LEN 4096

static int tcp_raw_recv(int socket, char *buffer);

char * to_tcp_packet_type(to_packet_type type){
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
	p->socket = 0;
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
		return -1;
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
		return -1;
	}
	
	freeaddrinfo(servInfo);
	
	if (listen(sock, BACKLOG) == -1) {
		to_log_err("listen() returned error ");
		return -1;
	}
	
	LOG_LEVEL0("Server listening");
	
	return sock;
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
		to_log_err("Failed to connect to server");
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
  	
	int n, numbytes, total_bytes = 0;
	char buffer[MAX_BUF_LEN];
	int bytes_left = packet->raw_data_len;

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
	
	/* not sure if timeout is necessary */
	const struct timeval timeout={.tv_sec=0, .tv_usec=800};
	setsockopt(packet->socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	if((numbytes = recv(packet->socket, buffer, MAX_BUF_LEN-1, 0)) == -1){
		to_log_err("Problem receiving data from server");
		return -1;
	}
	buffer[numbytes] = '\0';
	
	close(packet->socket);
	return total_bytes;
}

to_packet_t * to_tcp_read_packet(int socket){

	int len;
	char raw_data[MAX_BUF_LEN];
	to_packet_t * packet;
	
	packet = to_tcp_prep_packet();
	if(!packet) return NULL;
	
	len = tcp_raw_recv(socket, raw_data);
	if(!(len > 0)) return NULL;

	LOG_LEVEL1("Read [%d] bytes", len);
	packet->socket = socket;
	/* raw_data is already correctly '\0' terminated by @tcp_raw_recv */
	packet->raw_data = malloc(sizeof(char) * len);
	if(!packet->raw_data) return NULL;

	memcpy(packet->raw_data, raw_data, len);
	packet->raw_data_len = len;
	LOG_LEVEL2("Raw data received [%s]", packet->raw_data);
	
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
			to_log_err("Problem with recv()");
		}
	}
	
	return -1; /* something weird happened */
}
