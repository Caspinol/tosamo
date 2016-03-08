#include "include/master.h"

/* Master is essentially a client so it will initialize connections 
 * and send the files to the slave
 */


void mstr_send_update(void){

	L_HEAD *local_obj; //object file on this machine
	to_packet_t *response,
		*request = to_tcp_prep_packet();
	
	request->packet_type = PACKET_UPDATE;
	
	local_obj = obj_file_parse(main_settings.object_path, main_settings.tag, false);

	LOG_LEVEL1("Serializing data");
	if(to_data_serialize(request, local_obj) < 0){
		to_log_err("Error serializing data");
	}

	LOG_LEVEL3_RAW(request->raw_data, request->raw_data_len, "Serialized data dump");
	
	int socket = to_tcp_remote_connect(main_settings.remote_ip, main_settings.port);	
	if(socket < 0){
		to_log_err("Failed to connect to [%s]", main_settings.remote_ip);
	}
	request->socket = socket;	
	
	to_tcp_send_packet(request);

	LOG_LEVEL0("Awaiting slave confirmation...");
	/* Now wait if all was fine on the other side */
	response = to_tcp_read_packet(request->socket, true);
	if(to_data_deserialize(response)){
		to_log_err("Failure deserializing response");
		goto CLEANUP;
	}

	if(response->packet_type == PACKET_ACK){
		LOG_LEVEL0("Remote object file succesfully updated");
	}else{
		to_log_err("Failed to update remote object");
	}

 CLEANUP:
	to_list_destroy(local_obj);
	to_tcp_packet_destroy(&request);
	to_tcp_packet_destroy(&response);
}
