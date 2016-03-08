#include "include/slave.h"


/* Slave is like a server 
 * It will receive an connections from master 
 * with the updated pieces of object files
 */

static void *conn_handler(void *);

static pthread_t req_handler_th;
//static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void slave_handle_updates(volatile int *running){
	int server_sock;
	struct sockaddr_in client_ip;
	char client_ip_s[INET_ADDRSTRLEN];
	socklen_t salen;
	
	server_sock = to_tcp_listen(main_settings.my_ip, main_settings.port);
	if(server_sock < 0){
		to_log_err("Failed to set up network listener");
		return;
	}
	
	while(*running){

		int req_sock = accept(server_sock, (struct sockaddr *)&client_ip, &salen);
		if(0 <= req_sock){
			
			/* Show us who is connecting */
			getnameinfo((struct sockaddr *)&client_ip, sizeof(client_ip),
				    client_ip_s, INET_ADDRSTRLEN, NULL, 0, NI_NUMERICHOST);

			/* got request so handle it in a shiny, new, still worm thread */ 
			if(pthread_create(&req_handler_th, NULL, conn_handler, (void *)&req_sock) < 0){
				to_log_err("Something went wrong while creating connection handler thread");
				return;
			}
			
			/* dont wait for threads to finish */
			//pthread_detach(req_handler_th);

			/* ...ok maybe wait after all */
			pthread_join(req_handler_th, NULL);
		}
	}
	close(server_sock);
}

static void *conn_handler(void *client_sock){  

	L_HEAD * remote_obj = NULL;
	L_HEAD * local_obj = NULL;
	uint8_t chsum = 0;
	
	to_packet_t * request,
		    *response = to_tcp_prep_packet();

	int sock = *(int*) client_sock;
	
	request = to_tcp_read_packet(sock, false);
	response->socket = request->socket;
	
	LOG_LEVEL3_RAW(request->raw_data, request->raw_data_len,
		       "Raw bytes from socket ********");
	
	/* now we need to deserialize the data */
	if(to_data_deserialize(request)){
		to_log_err("Error deserializing raw bytes");
		goto CLEANUP;
	}

	LOG_LEVEL2("De-serialized data: [%s] for object file: [%s]",
		   request->obj_data, request->obj_path);

	chsum = crc(request->obj_data, request->obj_data_len);

	
	if(chsum != request->crc){
		to_log_err("CRC check failed.\nExpected [%d] but got [%d]",
			   request->crc, chsum);
		response->packet_type = PACKET_NACK;
		/* Let the master know something is NOT OK */
		to_data_serialize(response, NULL);
		to_tcp_send_packet(response);
		goto CLEANUP;
	}
	
	LOG_LEVEL0("CRC check OK!");
	response->packet_type = PACKET_ACK;
	/* Let the master know we OK */
	to_data_serialize(response, NULL);
	to_tcp_send_packet(response);
	
	
	remote_obj = obj_buffer_parse(request->obj_data, request->obj_data_len, main_settings.tag);
	local_obj = obj_file_parse(main_settings.object_path, main_settings.tag, true);
	if(!local_obj){
		to_log_err("Failed to read local [%s]",
			   main_settings.object_path);
		goto CLEANUP;
	}

	obj_file_replace_tagged_parts(local_obj, remote_obj);
	
	obj_write_to_file(main_settings.object_path, local_obj);
	
	LOG_LEVEL0("Updating [%s] with the latest data from [%s]",
		   main_settings.object_path, main_settings.remote_ip);

 CLEANUP:
	LOG_LEVEL1("Cleaning up thread data...");
	LOG_LEVEL2("Closing socket");
	close(sock);
	to_tcp_packet_destroy(&request);
	LOG_LEVEL2("Destroying linked lists content");
	to_list_destroy(remote_obj);
	to_list_destroy(local_obj);
	LOG_LEVEL2("...done. Exiting thread");
	pthread_exit(NULL);
}
