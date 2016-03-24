#include "include/tcp.h"

#define BACKLOG 10
#define MAX_BUF_LEN 8192

static int _tcp_raw_recv(int socket, char **buffer);

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
  	
	size_t n, total_bytes = 0;
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
		if(n == (size_t)-1){
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
	char *raw_data;
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
	
	len = _tcp_raw_recv(socket, &raw_data);
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
		free(raw_data);
		to_tcp_packet_destroy(&packet);
		return NULL;
	}
	
	return packet;
}

static int _tcp_raw_recv(int socket, char **buffer){
	int num;
	size_t total_numbytes = 0;
	size_t buf_alloc_size = MAX_BUF_LEN;
	char raw_rcv[MAX_BUF_LEN];

	*buffer = malloc(MAX_BUF_LEN);

	if(!*buffer){
		return -1;
	}
	while((num = recv(socket, raw_rcv, MAX_BUF_LEN, 0)) > 0){
		if(strstr(raw_rcv, "\x1f\x1f")){
			memcpy(*buffer + total_numbytes, raw_rcv, num);
			total_numbytes += num;
			return total_numbytes;
		}else{
			/* Double the buffer size */
			buf_alloc_size *= 2;
			
			char *r_tmp = realloc(*buffer, buf_alloc_size);
			if(!r_tmp){
				free(*buffer);
				return -1;
			}
			*buffer = r_tmp;

			memcpy(*buffer + total_numbytes, raw_rcv, num);
			total_numbytes += num;
		}
	}
	
	return -1; /* something weird happened */
}
