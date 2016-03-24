#include "serialize.h"

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

int to_data_serialize(to_packet_t * packet){
	
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

		return packet->raw_data_len;

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
#define FIND_COMMA() while(*(here++) != '\x1f' && (size_t)(here - packet->raw_data) < packet->raw_data_len);

int to_data_deserialize(to_packet_t * packet){

	size_t obj_len = 0;
	char *there;
	char *here = there = packet->raw_data;

	/* 
	   get type 
	   its only 1 byte
	*/
	packet->packet_type = *here;
	
	FIND_COMMA();

	switch(packet->packet_type){
	case PACKET_UPDATE:

		/* get fileneame */
		there = here;
		FIND_COMMA();
		if((here - there) < 1) return -1;
		
		memcpy(packet->obj_path, there, here - there - 1);
		packet->obj_path[here - there - 1] = '\0';
		LOG_LEVEL2("File recved [%s]", packet->obj_path);
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

		return here - there;

	case PACKET_ACK:
	case PACKET_NACK:
		/* Its ACK/NACK so not much to do now */
		return here - there;

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

		return here - there;
	}
	
	return -1;
}

#undef FIND_COMMA
