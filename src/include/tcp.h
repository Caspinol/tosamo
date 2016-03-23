#ifndef _TCP_H_
#define _TCP_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "settings.h"
#include "crc.h"
#include "log.h"

typedef enum to_packet_type {
	PACKET_UPDATE = 0,
	PACKET_ACK,
	PACKET_NACK,
	PACKET_CRC
}to_packet_type;

typedef struct to_packet_t {
	int socket;
	to_packet_type packet_type; /* Type can be as the enum above */
	char * raw_data; /* raw data send/recv over socket */
	size_t raw_data_len; /* len of the above */
	char obj_path[OBJ_PATH_LEN]; /* Name(path) of the file under sync */
	char * obj_data; /* unserialized data (just the object file bits) */
	size_t obj_data_len; /* len of the above */
	uint8_t crc; /* ...of unserialized data */
}to_packet_t;

/* serialize.h uses to_packet_t struct as argument */
#include "serialize.h"


char * to_tcp_packet_type(to_packet_type);
to_packet_t * to_tcp_prep_packet(void);
void to_tcp_packet_destroy(to_packet_t **);
int to_tcp_listen(char const *, char const *);
int to_tcp_accept(int socket);
int to_tcp_remote_connect(const char *, const char *);
int to_tcp_send_packet(to_packet_t *);
to_packet_t * to_tcp_read_packet(int, bool);

#endif
