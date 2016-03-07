#include "include/master.h"

/* Master is essentially a client so it will initialize connections 
 * and send the files to the slave
 */


void mstr_send_update(void){

	L_HEAD *local_obj; //object file on this machine
	to_packet_t * out_packet = to_tcp_prep_packet();
	out_packet->packet_type = PACKET_UPDATE;
	
	local_obj = obj_file_parse(main_settings.object_path, main_settings.tag, false);

	LOG_LEVEL1("Serializing data");
	if(to_data_serialize(out_packet, local_obj) < 0){
		to_log_err("Error serializing data");
	}

	LOG_LEVEL3_RAW(out_packet->raw_data, out_packet->raw_data_len, "Serialized data dump");
	
	int socket = to_tcp_remote_connect(main_settings.remote_ip, main_settings.port);	
	if(socket < 0){
		to_log_err("Failed to connect to [%s]", main_settings.remote_ip);
	}
	out_packet->socket = socket;	
	
	to_tcp_send_packet(out_packet);

	to_list_destroy(local_obj);
	to_tcp_packet_destroy(&out_packet);
}
