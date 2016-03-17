#include "include/master.h"

/* Master is essentially a client so it will initialize connections 
 * and send the files to the slave
 */

static void *timed_task_thread(void *);

static __thread unsigned char last_crc = 0;

static void do_update(void *file){
	
	L_HEAD *local_obj; //object file on this machine
	char *obj_data_buff = NULL;
	to_packet_t *response = NULL, *request = NULL;
	unsigned char this_crc = 0;
	
	local_obj = obj_file_parse((char *)file, main_settings.tag, false);
	if(!local_obj){
		to_log_err("Failure parsing the local object");
		return;
	}
	
	int obj_data_len = to_list_2_buf(local_obj, &obj_data_buff);	
	if(obj_data_len < 0){
		return;
	}
	
	/* Check crc if same as last one dont bother sending update */
	this_crc = crc(obj_data_buff, obj_data_len);
	if(this_crc == last_crc){
		LOG_LEVEL0("File unchanged. No update");
		free(obj_data_buff);
		to_list_destroy(local_obj);
		return;
	}
	LOG_LEVEL2("File has changed - sending update");
	
	int socket = to_tcp_remote_connect(main_settings.remote_ip, main_settings.port);	
	if(socket < 0){
		free(obj_data_buff);
		to_list_destroy(local_obj);
		return;
	}
	
	/* Update len and pointer to the obj_file buffer */
	request = to_tcp_prep_packet();
	request->packet_type = PACKET_UPDATE;
	strncpy(request->obj_path, file, strlen(file));
	request->obj_data = obj_data_buff;
	request->obj_data_len = obj_data_len;
	request->socket = socket;
	
	to_tcp_send_packet(request);
	
	LOG_LEVEL0("Awaiting slave confirmation...");

	/* Now wait if all was fine on the other side */
	response = to_tcp_read_packet(request->socket, true);

	if(response->packet_type == PACKET_ACK){
		LOG_LEVEL0("Remote object file succesfully updated");
		/* Update CRC only after sussesfull update */
		last_crc = this_crc;
	}else{
		to_log_err("Failed to update remote object");
	}

	to_list_destroy(local_obj);
	to_tcp_packet_destroy(&request);
	to_tcp_packet_destroy(&response);

	return;
}

static void *timed_task_thread(void *obj_path){

	char *jobname;
	asprintf(&jobname, "Update job for file %s", (char *)obj_path);

	to_timed_init_job(jobname, main_settings.scan_frequency);
	
	to_timed_run_periodic_job(do_update, obj_path);

	free(jobname);
	pthread_exit(NULL);
}

void master_monitor_files(void){

	pthread_t updater_th[main_settings.object_count];
	
	for(int i = 0; i < main_settings.object_count; i++){
		if(pthread_create(&updater_th[i], NULL,
				  timed_task_thread, (void *)main_settings.object_path[i]) < 0){
			to_log_err("Something went wrong while creating updater handler thread");
			return;
		}
	}

	for(int i = 0; i < main_settings.object_count; i++){
		pthread_join(updater_th[i], NULL);
	}
}
