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
        
	server_sock = to_tcp_listen(main_settings.my_ip, main_settings.port);
	if(server_sock < 0){
		to_log_err("Failed to set up network listener");
		return;
	}

	do{

		int req_sock = to_tcp_accept(server_sock);
		if(0 <= req_sock){
			
			/* got request so handle it in a shiny, new, still worm thread */ 
			if(pthread_create(&req_handler_th, NULL, conn_handler, (void *)&req_sock) < 0){
				to_log_err("Something went wrong while creating connection handler thread");
				return;
			}

			/* ...ok maybe wait after all */
			pthread_join(req_handler_th, NULL);
		}
	}while(*running);
	close(server_sock);
}

static void *conn_handler(void *client_sock){  

	L_HEAD * remote_obj = NULL;
	L_HEAD * local_obj = NULL;
	uint8_t chsum = 0;
	bool all_ok = true;
	
	to_packet_t * request = NULL, *response = to_tcp_prep_packet();

	int sock = *(int*) client_sock;
	/* Prepare the response packet */
	response->socket = sock;
	response->packet_type = PACKET_NACK;
	
	request = to_tcp_read_packet(sock, false);

	chsum = crc(request->obj_data, request->obj_data_len);
	if(chsum != request->crc){
		to_log_err("CRC check failed.\nExpected [%d] but got [%d]",
			   request->crc, chsum);
		all_ok = false;
		goto CLEANUP;
	}
	
	LOG_LEVEL1("CRC check OK!");
	
	remote_obj = obj_buffer_parse(request->obj_data, request->obj_data_len, main_settings.tag);
	if(!remote_obj){
		to_log_err("Failed to parse data from master");
		all_ok = false;
		goto CLEANUP;
	}

	local_obj = obj_file_parse(main_settings.object_path, main_settings.tag, true);
	if(!local_obj){
		to_log_err("Failed to read local [%s]", main_settings.object_path);
		all_ok = false;
		goto CLEANUP;
	}

	obj_file_replace_tagged_parts(local_obj, remote_obj);
	
	if(obj_write_to_file(main_settings.object_path, local_obj) < 0){
		to_log_err("Failed to write the chcnges to [%s]", main_settings.object_path);
		all_ok = false;
		goto CLEANUP;
	}
	
	LOG_LEVEL0("Updating [%s] with the latest data from [%s]",
		   main_settings.object_path, main_settings.remote_ip);

 CLEANUP:

	if(all_ok){
		response->packet_type = PACKET_ACK;
	}
	
	/* Let the master know we OK */
	to_tcp_send_packet(response);

	
	LOG_LEVEL1("Cleaning up thread data...");
	to_tcp_packet_destroy(&request);
	to_tcp_packet_destroy(&response);
	if(remote_obj) to_list_destroy(remote_obj);
	if(local_obj) to_list_destroy(local_obj);
	pthread_exit(NULL);
}
